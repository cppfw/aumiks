/* The MIT License:

Copyright (c) 2009-2011 Ivan Gagis

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE. */


#include <list>

#include <ting/debug.hpp>
#include <ting/types.hpp>
#include <ting/Thread.hpp>
#include <ting/Array.hpp>
#include <ting/Buffer.hpp>

#include "aumiks.hpp"

#include "../backend/PulseAudioBackend.hpp"
//#include "backend/ALSABackend.hpp"
//#include "backend/ESoundBackend.hpp"

using namespace aumiks;

Lib::Lib(unsigned bufferSizeMillis, aumiks::E_Format format) :
		thread(bufferSizeMillis, format)
{
	this->thread.Start();
}



Lib::~Lib(){
//	TRACE(<< "aumiks::~Lib(): enter" << std::endl)
	//need to stop thread before closing the device
	//because it can attempt write data after the device is closed
	this->thread.PushQuitMessage();
	this->thread.Join();
//	TRACE(<< "aumiks::~Lib(): exit" << std::endl)
}



void Lib::PlayChannel(ting::Ref<Channel> ch){
	ASSERT(ch.IsValid())

	//TODO: remove from here and add to mixing procedure
	if(this->thread.isMuted){
		return;
	}

	{
		ting::Mutex::Guard mut(this->thread.chPoolMutex);
		if(ch->IsPlaying())
			return;//already playing
		this->thread.chPoolToAdd.push_back(ch);//queue channel to be added to playing pool
		ch->isPlaying = true;//mark channel as playing
	}

	//in case the thread is hanging on the queue, wake it up by sending the nop message.
	this->thread.PushNopMessage();
}



Lib::SoundThread::SoundThread(unsigned bufferSizeMillis, E_Format format) :
		audioBackend(PulseAudioBackend::New(bufferSizeMillis, format)),
		mixerBuffer(SoundThread::CreateMixerBuffer(bufferSizeMillis, format))
{
//	TRACE(<< "SoundThread(): invoked" << std::endl)
}



namespace aumiks{

class MixerBuffer44100Stereo16 : public Lib::MixerBuffer{
	MixerBuffer44100Stereo16(unsigned bufferSizeMillis) :
			MixerBuffer(
					aumiks::Lib::BufferSizeInSamples(bufferSizeMillis, STEREO_16_44100), //size in s32
					aumiks::Lib::BufferSizeInSamples(bufferSizeMillis, STEREO_16_44100) * 2 //size in bytes
				)
	{}
	
	//override
	virtual bool MixToMixBuf(const ting::Ref<aumiks::Channel>& ch){
		ASSERT(ch.IsValid())
		return ch->MixToMixBuf44100Stereo16(this->mixBuf);
	}
	
	//override
	virtual void CopyFromMixBufToPlayBuf(){
		ASSERT((this->mixBuf.Size() * 2) == this->playBuf.Size())

		const ting::s32 *src = this->mixBuf.Begin();
		ting::u8* dst = this->playBuf.Begin();
		for(; src != this->mixBuf.End(); ++src, ++dst){
			ting::s32 tmp = *src;
			ting::ClampTop(tmp, 0x7fff);
			ting::ClampBottom(tmp, -0x7fff);

			ASSERT(dst < this->playBuf.End())
			*dst = ting::u8(tmp);
			++dst;
			ASSERT(dst < this->playBuf.End())
			*dst = ting::u8(tmp >> 8);
		}
	}
	
public:
	inline static ting::Ptr<MixerBuffer44100Stereo16> New(unsigned bufferSizeMillis){
		return ting::Ptr<MixerBuffer44100Stereo16>(
				new MixerBuffer44100Stereo16(bufferSizeMillis)
			);
	}
};



//TODO: add mixerBuffer classes for all formats
}



//static
ting::Ptr<Lib::MixerBuffer> Lib::SoundThread::CreateMixerBuffer(unsigned bufferSizeMillis, E_Format format){
	switch(format){
		//TODO:
		
		case aumiks::STEREO_16_44100:
			return MixerBuffer44100Stereo16::New(bufferSizeMillis);
		default:
			throw aumiks::Exc("Unknown sound output format requested");
	}
}



void Lib::SoundThread::Run(){
	M_AUMIKS_TRACE(<< "Thread started" << std::endl)
	

	while(!this->quitFlag){
		//If there is nothing to play then just hang on the message queue.
		//When new channel is added to the list a NOP message will be sent to
		//the queue. Otherwise, handle the pending messages if any and continue mixing.
		if(this->chPool.size() == 0){
			M_AUMIKS_TRACE(<< "SoundThread::Run(): going to hang on GetMsg()" << std::endl)
			this->queue.GetMsg()->Handle();
			M_AUMIKS_TRACE(<< "SoundThread::Run(): GetMsg() returned, message handled" << std::endl)
		}else{
			while(ting::Ptr<ting::Message> m = this->queue.PeekMsg()){
				m->Handle();
			}
		}

		//clean mixBuf
		this->mixerBuffer->CleanMixBuf();

		{//add queued channels to playing pool
			ting::Mutex::Guard mut(this->chPoolMutex);//lock mutex
			M_AUMIKS_TRACE(<< "chPoolToAdd.size() = " << this->chPoolToAdd.size() << std::endl)
			while(this->chPoolToAdd.size() != 0){
				ting::Ref<aumiks::Channel> ch = this->chPoolToAdd.front();
				this->chPoolToAdd.pop_front();
				this->chPool.push_back(ch);
				ch->OnStart();//notify channel that it was just started
			}
		}

		//mix channels to mixbuf
		for(TChIter i = this->chPool.begin(); i != this->chPool.end();){
			if(this->mixerBuffer->MixToMixBuf(*i)){
				(*i)->isPlaying = false;//clear playing flag
				i = this->chPool.erase(i);
			}else{
				++i;
			}
		}

//		TRACE(<< "chPool.size() = " << this->chPool.size() << std::endl)
		M_AUMIKS_TRACE(<< "mixed, copying to playbuf..." << std::endl)

		this->mixerBuffer->CopyFromMixBufToPlayBuf();

		M_AUMIKS_TRACE(<< "SoundThread::Run(): writing data..." << std::endl)

		//write data
		this->audioBackend->Write(this->mixerBuffer->playBuf);
	}//~while

//	TRACE(<< "SoundThread::Run(): exiting" << std::endl)
}



unsigned Lib::BufferSizeInSamples(unsigned bufferSizeMillis, E_Format format){
	unsigned samplesPerSecond;
	
	switch(format){
		case MONO_16_11025:
			samplesPerSecond = 11025;
			break;
		case STEREO_16_11025:
			samplesPerSecond = 2 * 11025;
			break;
		case MONO_16_22050:
			samplesPerSecond = 22050;
			break;
		case STEREO_16_22050:
			samplesPerSecond = 2 * 22050;
			break;
		case MONO_16_44100:
			samplesPerSecond = 44100;
			break;
		case STEREO_16_44100:
			samplesPerSecond = 2 * 44100;
			break;
		default:
			throw aumiks::Exc("unknown sound format");
	}
	
	return samplesPerSecond * bufferSizeMillis / 1000;
}

