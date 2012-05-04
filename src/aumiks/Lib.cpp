/* The MIT License:

Copyright (c) 2012 Ivan Gagis

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

// Home page: http://aumiks.googlecode.com


#include <ting/config.hpp>

#include "Lib.hpp"


using namespace aumiks;



ting::IntrusiveSingleton<Lib>::T_Instance Lib::instance;



#include "backend/AudioBackend.hpp"

#if M_OS == M_OS_WIN32 || M_OS == M_OS_WIN64
#	include "backend/DirectSoundBackend.hpp"
#elif M_OS == M_OS_LINUX
#	if defined(__ANDROID__)
#		include "backend/OpenSLESBackend.hpp"
#	else
#		include "backend/PulseAudioBackend.hpp"
//#		include "backend/ALSABackend.hpp"
#	endif
#else
#	error "Unknown OS"
#endif



Lib::Lib(ting::u16 bufferSizeMillis, unsigned freq, unsigned chans) :
		freq(freq),
		chans(chans),
		bufSizeInFrames(freq * bufferSizeMillis / 1000),
		mixChannel(aumiks::MixChannel::New()),
		smpBuf(bufSizeInFrames * chans)
{
	//backend must be initialized after all the essential parts of aumiks are initialized,
	//because after the backend object is created, it starts calling the FillPlayBuf() method periodically.
	this->backend = reinterpret_cast<void*>(static_cast<AudioBackend*>(
#if M_OS == M_OS_WIN32 || M_OS == M_OS_WIN64
			new DirectSoundBackend(this->bufSizeInFrames, freq, chans)
#elif M_OS == M_OS_LINUX
#	if defined(__ANDROID__)
			new OpenSLESBackend(this->bufSizeInFrames, freq, chans)
#	else
			new PulseAudioBackend(this->bufSizeInFrames, freq, chans)
//			new ALSABackend(this->bufSizeInFrames, freq, chans)
#	endif
#else
#	error "undefined OS"
#endif
		));
}



Lib::~Lib()throw(){
	delete reinterpret_cast<AudioBackend*>(this->backend);
}



void Lib::PlayChannel_ts(const ting::Ref<Channel>& ch){
	ASSERT(ch.IsValid())

	{
		ting::Mutex::Guard mut(this->mutex);
		if(ch->IsPlaying()){
			return;//already playing
		}

		ch->InitEffects();

		this->channelsToAdd.push_back(ch);//queue channel to be added to playing pool
		ch->isPlaying = true;//mark channel as playing
		ch->soundStopped = false;//init sound stopped flag
		ch->stopFlag = false;
	}
}



void aumiks::Lib::FillPlayBuf(ting::Buffer<ting::u8>& playBuf){
	//Check matching of mixBuf size and playBuf size, 16 bits per sample
	ASSERT_INFO(
			this->mixerBuffer->mixBuf.Size() * 2 == playBuf.Size(),
			"playBuf.Size() = " << playBuf.Size() << " mixBuf.Size() = " << this->mixerBuffer->mixBuf.Size()
		)

	//TODO:
	
	//clean mixBuf
//	this->mixerBuffer->CleanMixBuf();

//	{//add queued channels to playing pool and effects to channels
//		ting::Mutex::Guard mut(this->mutex);//lock mutex
//
//		M_AUMIKS_TRACE(<< "chPoolToAdd.size() = " << this->channelsToAdd.size() << std::endl)
//		while(this->channelsToAdd.size() != 0){
//			ting::Ref<aumiks::Channel> ch = this->channelsToAdd.front();
//			this->channelsToAdd.pop_front();
//			this->chPool.push_back(ch);
//			ch->OnStart();//notify channel that it was just started
//		}
//
//		//add effects to channels
//		while(this->effectsToAddToChan.size() != 0){
//			T_ChannelEffectPair& p = this->effectsToAddToChan.front();
//			ASSERT(p.first)
//			ASSERT(p.second)
//
//			aumiks::Lib::AddEffectToChannelSync(p.first, p.second);
//
//			this->effectsToAddToChan.pop_front();
//		}
//
//		//remove effects from channels
//		while(this->effectsToRemoveFromChan.size() != 0){
//			T_ChannelEffectPair &p = this->effectsToRemoveFromChan.front();
//			ASSERT(p.first)
//			ASSERT(p.second)
//
//			aumiks::Lib::RemoveEffectFromChannelSync(p.first, p.second);
//
//			this->effectsToRemoveFromChan.pop_front();
//		}
//
//		//remove all effects from channels
//		while(this->effectsToClear.size() != 0){
//			this->effectsToClear.front()->effects.clear();
//			this->effectsToClear.pop_front();
//		}
//
//		//add global effects
//		while(this->effectsToAdd.size() != 0){
//			this->effects.push_back(this->effectsToAdd.front());
//			this->effectsToAdd.pop_front();
//		}
//
//		//remove global effects
//		while(this->effectsToRemove.size() != 0){
//			for(Effect::T_EffectsIter i = this->effects.begin(); i != this->effects.end(); ++i){
//				if(*i == this->effectsToRemove.front()){
//					this->effects.erase(i);
//					break;
//				}
//			}
//
//			this->effectsToRemove.pop_front();
//		}
//
//		//clear global effects if requested
//		if(this->clearEffects){
//			this->effects.clear();
//			this->clearEffects = false;
//		}
//	}//~mutex guard

		
	//mix channels to smpBuf
	this->mixChannel->FillSmpBufAndApplyEffects(this->smpBuf, this->freq, this->chans);

//		TRACE(<< "chPool.size() = " << this->chPool.size() << std::endl)
	M_AUMIKS_TRACE(<< "mixed, copying to playbuf..." << std::endl)

	this->CopySmpBufToPlayBuf(playBuf);
}



void aumiks::Lib::CopySmpBufToPlayBuf(ting::Buffer<ting::u8>& playBuf){
	//playBuf size is in bytes and we have 2 bytes per sample always.
	ASSERT((this->mixBuf.Size() * 2) == playBuf.Size())

	const ting::s32 *src = this->smpBuf.Begin();
	ting::u8* dst = playBuf.Begin();
	for(; src != this->smpBuf.End(); ++src){
		ting::s32 tmp = *src;
		ting::util::ClampTop(tmp, 0x7fff);
		ting::util::ClampBottom(tmp, -0x7fff);

		ASSERT(playBuf.Begin() <= dst && dst <= playBuf.End() - 1)
		*dst = ting::u8(tmp);
		++dst;
		ASSERT(playBuf.Begin() <= dst && dst <= playBuf.End() - 1)
		*dst = ting::u8(tmp >> 8);
		++dst;
	}
	ASSERT(dst == playBuf.End())
}
