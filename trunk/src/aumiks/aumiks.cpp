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

#include "../backend/AudioBackend.hpp"

#include "../backend/PulseAudioBackend.hpp"
//#include "backend/ALSABackend.hpp"
//#include "backend/ESoundBackend.hpp"

using namespace aumiks;

Lib::Lib(unsigned requestedBufferSizeInFrames) :
		thread(requestedBufferSizeInFrames)
{
	this->thread.Start();
}



Lib::~Lib(){
//	TRACE(<< "aumiks::~Lib(): enter")
	//need to stop thread before closing the device
	//because it can attempt write data after the device is closed
	this->thread.PushQuitMessage();
	this->thread.Join();
//	TRACE(<< "aumiks::~Lib(): exit")
}



void Lib::PlayChannel(ting::Ref<Channel> ch){
	ASSERT(ch.IsValid())

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



Lib::SoundThread::SoundThread(unsigned requestedBufferSizeInFrames) :
		desiredBufferSizeInFrames(requestedBufferSizeInFrames)
{
	TRACE(<< "SoundThread(): invoked" << std::endl)
}



static void CopyFromMixBufToPlayBuf(const ting::Array<ting::s32>& mixBuf, ting::Array<ting::u8>& playBuf){
	ASSERT(mixBuf.Size() == playBuf.Size() / 2) //2 bytes per sample

	//TODO: make it work for other bytes per sample values?

	const ting::s32 *src = mixBuf.Begin();
	ting::s16* dst = reinterpret_cast<ting::s16*>(playBuf.Begin());//TODO: cast from u8* to s16* is forbidden by c++ standard
	ASSERT(reinterpret_cast<void*>(dst) == reinterpret_cast<void*>(playBuf.Begin()))
	for(unsigned i = 0; i < mixBuf.Size(); ++i){
		ting::s32 tmp = *src;
		ting::ClampTop(tmp, 0x7fff);
		ting::ClampBottom(tmp, -0x7fff);

		*dst = ting::s16(tmp);
		++dst;
		++src;
	}
}



void Lib::SoundThread::Run(){
	M_AUMIKS_TRACE(<< "Thread started" << std::endl)

	ting::Ptr<aumiks::AudioBackend> backend(new PulseAudioBackend(this->desiredBufferSizeInFrames));
//	ting::Ptr<aumiks::AudioBackend> backend(new ALSABackend(this->desiredBufferSizeInFrames));
//	ting::Ptr<aumiks::AudioBackend> backend(new ESoundBackend(this->desiredBufferSizeInFrames));

	ting::Array<ting::s32> mixBuf(backend->BufferSizeInSamples());
	ting::Array<ting::u8> playBuf(backend->BufferSizeInBytes());

	M_AUMIKS_TRACE(<< "mixBuf.Size() = " << mixBuf.Size() << std::endl)

	while(!this->quitFlag){
		while(ting::Ptr<ting::Message> m = this->queue.PeekMsg()){
			m->Handle();
		}

		//clean mixBuf
		memset(mixBuf.Begin(), 0, mixBuf.SizeInBytes());

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

		if(this->isMuted){
			this->chPool.clear();
		}

		//If there is nothing to play then just hang on the message queue.
		//When new channel is added to the list a NOP message will be sent to
		//the queue.
		if(this->chPool.size() == 0){
			this->queue.GetMsg()->Handle();
			continue;
		}

		//mix channels to mixbuf
		for(TChIter i = this->chPool.begin(); i != this->chPool.end();){
			if((*i)->MixToMixBuf(mixBuf)){
				(*i)->isPlaying = false;//clear playing flag
				i = this->chPool.erase(i);
			}else{
				++i;
			}
		}

//		TRACE(<< "chPool.size() = " << this->chPool.size() << std::endl)
		M_AUMIKS_TRACE(<< "mixed, copying to playbuf..." << std::endl)

		CopyFromMixBufToPlayBuf(mixBuf, playBuf);

		M_AUMIKS_TRACE(<< "SoundThread::Run(): writing data..." << std::endl)

		//write data
		backend->Write(playBuf);
	}//~while

	TRACE(<< "SoundThread::Run(): exiting" << std::endl)
}


