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



namespace{

unsigned BufferSizeInFrames(unsigned bufferSizeMillis, E_Format format){
	unsigned framesPerSecond;
	
	//NOTE: for simplicity of conversion from lower sample rates to higher ones,
	//      the size of output buffer should be that it would hold no fractional
	//      parts of source samples when they are mixed to it.
	unsigned granularity;
	
	switch(format){
		case MONO_16_11025:
		case STEREO_16_11025:
			framesPerSecond = 11025;
			granularity = 1;
			break;
		case MONO_16_22050:
		case STEREO_16_22050:
			framesPerSecond = 22050;
			granularity = 2;
			break;
		case MONO_16_44100:
		case STEREO_16_44100:
			framesPerSecond = 44100;
			granularity = 4;
			break;
		default:
			throw aumiks::Exc("unknown sound format");
	}
	
	unsigned ret = framesPerSecond * bufferSizeMillis / 1000;
	return ret + (granularity - ret % granularity);
}



unsigned BufferSizeInSamples(unsigned bufferSizeMillis, E_Format format){
	return BufferSizeInFrames(bufferSizeMillis, format) * aumiks::SamplesPerFrame(format);
}

}//~namespace



Lib::Lib(ting::u16 bufferSizeMillis, aumiks::E_Format format) :
		mixerBuffer(Lib::CreateMixerBuffer(BufferSizeInSamples(bufferSizeMillis, format), format)),
		audioBackend(PulseAudioBackend::New(BufferSizeInFrames(bufferSizeMillis, format), format))
{}



void Lib::AddEffectToChannel_ts(const ting::Ref<Channel>& ch, const ting::Ref<aumiks::Effect>& eff){
	ASSERT(ch.IsValid())

	{
		ting::Mutex::Guard mut(this->chPoolMutex);
		
		this->effectsToAdd.push_back(T_ChannelEffectPair(ch, eff));//queue channel to be added to playing pool
	}
}



void Lib::RemoveEffectFromChannel_ts(const ting::Ref<Channel>& ch, const ting::Ref<aumiks::Effect>& eff){
	ASSERT(ch.IsValid())

	{
		ting::Mutex::Guard mut(this->chPoolMutex);
		
		this->effectsToRemove.push_back(T_ChannelEffectPair(ch, eff));//queue channel to be added to playing pool
	}
}



void Lib::PlayChannel_ts(const ting::Ref<Channel>& ch){
	ASSERT(ch.IsValid())

	{
		ting::Mutex::Guard mut(this->chPoolMutex);
		if(ch->IsPlaying())
			return;//already playing
		
		ch->InitEffects();
		
		this->chPoolToAdd.push_back(ch);//queue channel to be added to playing pool
		ch->isPlaying = true;//mark channel as playing
		ch->soundStopped = false;//init sound stopped flag
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
	
	//call channel effects
	bool ret = this->ApplyEffectsToSmpBuf(ch);
	ret &= ch->soundStopped;
	
	if(this->isMuted){
		return ret;
	}
	
	this->MixSmpBufToMixBuf();
	return ret;
}



namespace{

class MixerBuffer11025Mono16 : public Lib::MixerBuffer{
	MixerBuffer11025Mono16(unsigned bufferSizeInSamples) :
			MixerBuffer(bufferSizeInSamples)
	{}
	
	//override
	virtual bool FillSmpBuf(const ting::Ref<aumiks::Channel>& ch){
		return this->FillSmpBuf11025Mono16(ch);
	}
	
	//override
	virtual bool ApplyEffectsToSmpBuf(const ting::Ref<aumiks::Channel>& ch){
		return this->ApplyEffectsToSmpBuf11025Mono16(ch);
	}
	
public:
	inline static ting::Ptr<MixerBuffer11025Mono16> New(unsigned bufferSizeInSamples){
		return ting::Ptr<MixerBuffer11025Mono16>(
				new MixerBuffer11025Mono16(bufferSizeInSamples)
			);
	}
};



class MixerBuffer11025Stereo16 : public Lib::MixerBuffer{
	MixerBuffer11025Stereo16(unsigned bufferSizeInSamples) :
			MixerBuffer(bufferSizeInSamples)
	{}
	
	//override
	virtual bool FillSmpBuf(const ting::Ref<aumiks::Channel>& ch){
		return this->FillSmpBuf11025Stereo16(ch);
	}
	
	//override
	virtual bool ApplyEffectsToSmpBuf(const ting::Ref<aumiks::Channel>& ch){
		return this->ApplyEffectsToSmpBuf11025Stereo16(ch);
	}
public:
	inline static ting::Ptr<MixerBuffer11025Stereo16> New(unsigned bufferSizeInSamples){
		return ting::Ptr<MixerBuffer11025Stereo16>(
				new MixerBuffer11025Stereo16(bufferSizeInSamples)
			);
	}
};



class MixerBuffer22050Mono16 : public Lib::MixerBuffer{
	MixerBuffer22050Mono16(unsigned bufferSizeInSamples) :
			MixerBuffer(bufferSizeInSamples)
	{}
	
	//override
	virtual bool FillSmpBuf(const ting::Ref<aumiks::Channel>& ch){
		return this->FillSmpBuf22050Mono16(ch);
	}
	
	//override
	virtual bool ApplyEffectsToSmpBuf(const ting::Ref<aumiks::Channel>& ch){
		return this->ApplyEffectsToSmpBuf22050Mono16(ch);
	}
public:
	inline static ting::Ptr<MixerBuffer22050Mono16> New(unsigned bufferSizeInSamples){
		return ting::Ptr<MixerBuffer22050Mono16>(
				new MixerBuffer22050Mono16(bufferSizeInSamples)
			);
	}
};



class MixerBuffer22050Stereo16 : public Lib::MixerBuffer{
	MixerBuffer22050Stereo16(unsigned bufferSizeInSamples) :
			MixerBuffer(bufferSizeInSamples)
	{}
	
	//override
	virtual bool FillSmpBuf(const ting::Ref<aumiks::Channel>& ch){
		return this->FillSmpBuf22050Stereo16(ch);
	}
	
	//override
	virtual bool ApplyEffectsToSmpBuf(const ting::Ref<aumiks::Channel>& ch){
		return this->ApplyEffectsToSmpBuf22050Stereo16(ch);
	}
public:
	inline static ting::Ptr<MixerBuffer22050Stereo16> New(unsigned bufferSizeInSamples){
		return ting::Ptr<MixerBuffer22050Stereo16>(
				new MixerBuffer22050Stereo16(bufferSizeInSamples)
			);
	}
};



class MixerBuffer44100Mono16 : public Lib::MixerBuffer{
	MixerBuffer44100Mono16(unsigned bufferSizeInSamples) :
			MixerBuffer(bufferSizeInSamples)
	{}
	
	//override
	virtual bool FillSmpBuf(const ting::Ref<aumiks::Channel>& ch){
		return this->FillSmpBuf44100Mono16(ch);
	}
	
	//override
	virtual bool ApplyEffectsToSmpBuf(const ting::Ref<aumiks::Channel>& ch){
		return this->ApplyEffectsToSmpBuf44100Mono16(ch);
	}
public:
	inline static ting::Ptr<MixerBuffer44100Mono16> New(unsigned bufferSizeInSamples){
		return ting::Ptr<MixerBuffer44100Mono16>(
				new MixerBuffer44100Mono16(bufferSizeInSamples)
			);
	}
};



class MixerBuffer44100Stereo16 : public Lib::MixerBuffer{
	MixerBuffer44100Stereo16(unsigned bufferSizeInSamples) :
			MixerBuffer(bufferSizeInSamples)
	{}
	
	//override
	virtual bool FillSmpBuf(const ting::Ref<aumiks::Channel>& ch){
		return this->FillSmpBuf44100Stereo16(ch);
	}
	
	//override
	virtual bool ApplyEffectsToSmpBuf(const ting::Ref<aumiks::Channel>& ch){
		return this->ApplyEffectsToSmpBuf44100Stereo16(ch);
	}
public:
	inline static ting::Ptr<MixerBuffer44100Stereo16> New(unsigned bufferSizeInSamples){
		return ting::Ptr<MixerBuffer44100Stereo16>(
				new MixerBuffer44100Stereo16(bufferSizeInSamples)
			);
	}
};

}//~namespace



//static
ting::Ptr<Lib::MixerBuffer> Lib::CreateMixerBuffer(unsigned bufferSizeInSamples, E_Format format){
	switch(format){
		case aumiks::MONO_16_11025:
			return MixerBuffer11025Mono16::New(bufferSizeInSamples);
		case aumiks::STEREO_16_11025:
			return MixerBuffer11025Stereo16::New(bufferSizeInSamples);
		case aumiks::MONO_16_22050:
			return MixerBuffer22050Mono16::New(bufferSizeInSamples);
		case aumiks::STEREO_16_22050:
			return MixerBuffer22050Stereo16::New(bufferSizeInSamples);
		case aumiks::MONO_16_44100:
			return MixerBuffer44100Mono16::New(bufferSizeInSamples);
		case aumiks::STEREO_16_44100:
			return MixerBuffer44100Stereo16::New(bufferSizeInSamples);
		default:
			throw aumiks::Exc("Unknown sound output format requested");
	}
}



void aumiks::Lib::FillPlayBuf_ts(ting::Buffer<ting::u8>& playBuf){
	//Check matching of mixBuf size and playBuf size, 16 bits per sample
	ASSERT_INFO(
			this->mixerBuffer->mixBuf.Size() * 2 == playBuf.Size(),
			"playBuf.Size() = " << playBuf.Size() << " mixBuf.Size() = " << this->mixerBuffer->mixBuf.Size()
		)
	
	//clean mixBuf
	this->mixerBuffer->CleanMixBuf();

	{//add queued channels to playing pool and effects to channels
		ting::Mutex::Guard mut(this->chPoolMutex);//lock mutex

		M_AUMIKS_TRACE(<< "chPoolToAdd.size() = " << this->chPoolToAdd.size() << std::endl)
		while(this->chPoolToAdd.size() != 0){
			ting::Ref<aumiks::Channel> ch = this->chPoolToAdd.front();
			this->chPoolToAdd.pop_front();
			this->chPool.push_back(ch);
			ch->OnStart();//notify channel that it was just started
		}

		//add effects to channels
		while(this->effectsToAdd.size() != 0){
			T_ChannelEffectPair& p = this->effectsToAdd.front();
			ASSERT(p.first)
			ASSERT(p.second)

			aumiks::Lib::AddEffectToChannelSync(p.first, p.second);

			this->effectsToAdd.pop_front();
		}

		//remove effects from channels
		while(this->effectsToRemove.size() != 0){
			T_ChannelEffectPair &p = this->effectsToRemove.front();
			ASSERT(p.first)
			ASSERT(p.second)

			aumiks::Lib::RemoveEffectFromChannelSync(p.first, p.second);

			this->effectsToRemove.pop_front();
		}
	}

	//mix channels to mixbuf
	for(T_ChIter i = this->chPool.begin(); i != this->chPool.end();){
		if(this->mixerBuffer->MixToMixBuf(*i)){
			(*i)->isPlaying = false;//clear playing flag
			(*i)->OnStop();//notify channel that it has stopped playing
			i = this->chPool.erase(i);
		}else{
			++i;
		}
	}

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
		ting::ClampTop(tmp, 0x7fff);
		ting::ClampBottom(tmp, -0x7fff);

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

