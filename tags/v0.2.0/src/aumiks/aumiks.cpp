/* The MIT License:

Copyright (c) 2009-2012 Ivan Gagis

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



#include <list>

#include <ting/config.hpp>
#include <ting/debug.hpp>
#include <ting/types.hpp>
#include <ting/Thread.hpp>
#include <ting/Array.hpp>
#include <ting/Buffer.hpp>

#include "aumiks.hpp"

#if M_OS == M_OS_WIN32 || M_OS == M_OS_WIN64
	#include "backend/DirectSoundBackend.hpp"
#elif M_OS == M_OS_LINUX
	#if defined(__ANDROID__)
		#include "backend/OpenSLESBackend.hpp"
	#else
		#include "backend/PulseAudioBackend.hpp"
//		#include "backend/ALSABackend.hpp"
	#endif
#else
	#error "Unknown OS"
#endif


using namespace aumiks;



ting::IntrusiveSingleton<Lib>::T_Instance Lib::instance;



namespace{

unsigned BufferSizeInFrames(unsigned bufferSizeMillis, E_Format format){
	//NOTE: for simplicity of conversion from lower sample rates to higher ones,
	//      the size of output buffer should be that it would hold no fractional
	//      parts of source samples when they are mixed to it.
	unsigned granularity;

	switch(format){
		case MONO_16_11025:
		case STEREO_16_11025:
			granularity = 1;
			break;
		case MONO_16_22050:
		case STEREO_16_22050:
			granularity = 2;
			break;
		case MONO_16_44100:
		case STEREO_16_44100:
			granularity = 4;
			break;
		default:
			throw aumiks::Exc("unknown sound format");
	}

	unsigned ret = aumiks::SamplingRate(format) * bufferSizeMillis / 1000;
	return ret + (granularity - ret % granularity);
}



unsigned BufferSizeInSamples(unsigned bufferSizeMillis, E_Format format){
	return BufferSizeInFrames(bufferSizeMillis, format) * aumiks::SamplesPerFrame(format);
}

}//~namespace



Lib::Lib(ting::u16 bufferSizeMillis, aumiks::E_Format format) :
		mixerBuffer(Lib::CreateMixerBuffer(BufferSizeInSamples(bufferSizeMillis, format), format)),
#ifdef WIN32
		audioBackend(DirectSoundBackend::New(BufferSizeInFrames(bufferSizeMillis, format), format))
#elif defined(__linux__)
 #if defined(__ANDROID__)
		audioBackend(OpenSLESBackend::New(BufferSizeInFrames(bufferSizeMillis, format), format))
 #else
		audioBackend(PulseAudioBackend::New(BufferSizeInFrames(bufferSizeMillis, format), format))
//		audioBackend(ALSABackend::New(BufferSizeInFrames(bufferSizeMillis, format), format))
 #endif
#else
#error "undefined OS"
#endif
{}



void Lib::AddEffectToChannel_ts(const ting::Ref<Channel>& ch, const ting::Ref<aumiks::Effect>& eff){
	ASSERT(ch.IsValid())

	{
		ting::Mutex::Guard mut(this->mutex);

		this->effectsToAddToChan.push_back(T_ChannelEffectPair(ch, eff));//queue channel for affect addition
	}
}



void Lib::RemoveEffectFromChannel_ts(const ting::Ref<Channel>& ch, const ting::Ref<aumiks::Effect>& eff){
	ASSERT(ch.IsValid())

	{
		ting::Mutex::Guard mut(this->mutex);

		this->effectsToRemoveFromChan.push_back(T_ChannelEffectPair(ch, eff));//queue channel for single effect removal
	}
}



void Lib::RemoveAllEffectsFromChannel_ts(const ting::Ref<Channel>& ch){
	ASSERT(ch.IsValid())

	{
		ting::Mutex::Guard mut(this->mutex);

		this->effectsToClear.push_back(ch);//queue channel for removal of all effects
	}
}



void Lib::PlayChannel_ts(const ting::Ref<Channel>& ch){
	ASSERT(ch.IsValid())

	{
		ting::Mutex::Guard mut(this->mutex);
		if(ch->IsPlaying())
			return;//already playing

		ch->InitEffects();

		this->channelsToAdd.push_back(ch);//queue channel to be added to playing pool
		ch->isPlaying = true;//mark channel as playing
		ch->soundStopped = false;//init sound stopped flag
		ch->stopFlag = false;
	}
}



bool Lib::MixerBuffer::MixToMixBuf(const ting::Ref<aumiks::Channel>& ch){
	if(ch->stopFlag){
		return true;
	}

	if(!ch->soundStopped){
		ch->soundStopped = this->FillSmpBuf(ch);
	}else{
		//clear smp buffer
		memset(this->smpBuf.Begin(), 0, this->smpBuf.SizeInBytes());
	}

	//Call channel effects.
	//If there are no any effects, it should return ch->soundStopped, otherwise, depending on the effects.
	bool ret = this->ApplyEffectsToSmpBuf(ch);

	if(this->isMuted){
		return ret;
	}

	this->MixSmpBufToMixBuf();
	return ret;
}



//static
ting::Ptr<Lib::MixerBuffer> Lib::CreateMixerBuffer(unsigned bufferSizeInSamples, E_Format format){
	switch(format){
		case aumiks::MONO_16_11025:
			return MixerBufferImpl<11025, 1>::New(bufferSizeInSamples);
		case aumiks::STEREO_16_11025:
			return MixerBufferImpl<11025, 2>::New(bufferSizeInSamples);
		case aumiks::MONO_16_22050:
			return MixerBufferImpl<22050, 1>::New(bufferSizeInSamples);
		case aumiks::STEREO_16_22050:
			return MixerBufferImpl<22050, 2>::New(bufferSizeInSamples);
		case aumiks::MONO_16_44100:
			return MixerBufferImpl<44100, 1>::New(bufferSizeInSamples);
		case aumiks::STEREO_16_44100:
			return MixerBufferImpl<44100, 2>::New(bufferSizeInSamples);
		default:
			throw aumiks::Exc("Unknown sound output format requested");
	}
}



void aumiks::Lib::FillPlayBuf(ting::Buffer<ting::u8>& playBuf){
	//Check matching of mixBuf size and playBuf size, 16 bits per sample
	ASSERT_INFO(
			this->mixerBuffer->mixBuf.Size() * 2 == playBuf.Size(),
			"playBuf.Size() = " << playBuf.Size() << " mixBuf.Size() = " << this->mixerBuffer->mixBuf.Size()
		)

	//clean mixBuf
	this->mixerBuffer->CleanMixBuf();

	{//add queued channels to playing pool and effects to channels
		ting::Mutex::Guard mut(this->mutex);//lock mutex

		M_AUMIKS_TRACE(<< "chPoolToAdd.size() = " << this->channelsToAdd.size() << std::endl)
		while(this->channelsToAdd.size() != 0){
			ting::Ref<aumiks::Channel> ch = this->channelsToAdd.front();
			this->channelsToAdd.pop_front();
			this->chPool.push_back(ch);
			ch->OnStart();//notify channel that it was just started
		}

		//add effects to channels
		while(this->effectsToAddToChan.size() != 0){
			T_ChannelEffectPair& p = this->effectsToAddToChan.front();
			ASSERT(p.first)
			ASSERT(p.second)

			aumiks::Lib::AddEffectToChannelSync(p.first, p.second);

			this->effectsToAddToChan.pop_front();
		}

		//remove effects from channels
		while(this->effectsToRemoveFromChan.size() != 0){
			T_ChannelEffectPair &p = this->effectsToRemoveFromChan.front();
			ASSERT(p.first)
			ASSERT(p.second)

			aumiks::Lib::RemoveEffectFromChannelSync(p.first, p.second);

			this->effectsToRemoveFromChan.pop_front();
		}

		//remove all effects from channels
		while(this->effectsToClear.size() != 0){
			this->effectsToClear.front()->effects.clear();
			this->effectsToClear.pop_front();
		}

		//add global effects
		while(this->effectsToAdd.size() != 0){
			this->effects.push_back(this->effectsToAdd.front());
			this->effectsToAdd.pop_front();
		}

		//remove global effects
		while(this->effectsToRemove.size() != 0){
			for(Effect::T_EffectsIter i = this->effects.begin(); i != this->effects.end(); ++i){
				if(*i == this->effectsToRemove.front()){
					this->effects.erase(i);
					break;
				}
			}

			this->effectsToRemove.pop_front();
		}

		//clear global effects if requested
		if(this->clearEffects){
			this->effects.clear();
			this->clearEffects = false;
		}
	}//~mutex guard

	//mix channels to mixbuf
	for(T_ChannelIter i = this->chPool.begin(); i != this->chPool.end();){
		if(this->mixerBuffer->MixToMixBuf(*i)){
			(*i)->isPlaying = false;//clear playing flag
			(*i)->OnStop();//notify channel that it has stopped playing
			i = this->chPool.erase(i);
		}else{
			++i;
		}
	}

	//apply global effects
	this->mixerBuffer->ApplyEffectsToMixBuf();

//		TRACE(<< "chPool.size() = " << this->chPool.size() << std::endl)
	M_AUMIKS_TRACE(<< "mixed, copying to playbuf..." << std::endl)

	this->mixerBuffer->CopyToPlayBuf(playBuf);
}


void aumiks::Lib::MixerBuffer::MixSmpBufToMixBuf(){
	ASSERT(this->smpBuf.Size() == this->mixBuf.Size())

	ting::s32* src = this->smpBuf.Begin();
	ting::s32* dst = this->mixBuf.Begin();

	for(; dst != this->mixBuf.End(); ++src, ++dst){
		*dst += *src;
	}
}



void aumiks::Lib::MixerBuffer::CopyToPlayBuf(ting::Buffer<ting::u8>& playBuf){
	//playBuf size is in bytes and we have 2 bytes per sample always.
	ASSERT((this->mixBuf.Size() * 2) == playBuf.Size())

	const ting::s32 *src = this->mixBuf.Begin();
	ting::u8* dst = playBuf.Begin();
	for(; src != this->mixBuf.End(); ++src){
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



unsigned aumiks::BytesPerFrame(E_Format format){
	return 2 * SamplesPerFrame(format); //we only support 16 bits per sample
}


unsigned aumiks::SamplesPerFrame(E_Format format){
	switch(format){
		case aumiks::MONO_16_11025:
		case aumiks::MONO_16_22050:
		case aumiks::MONO_16_44100:
			return 1;
		case aumiks::STEREO_16_11025:
		case aumiks::STEREO_16_22050:
		case aumiks::STEREO_16_44100:
			return 2;
		default:
			throw aumiks::Exc("Unknown sound output format");
	}
}

unsigned aumiks::SamplingRate(E_Format format){
	switch(format){
		case aumiks::MONO_16_11025:
		case aumiks::STEREO_16_11025:
			return 11025;
		case aumiks::MONO_16_22050:
		case aumiks::STEREO_16_22050:
			return 22050;
		case aumiks::MONO_16_44100:
		case aumiks::STEREO_16_44100:
			return 44100;
		default:
			throw aumiks::Exc("Unknown sound output format");
	}
}
