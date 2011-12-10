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

/**
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once

#include <ting/types.hpp>
#include <ting/Singleton.hpp>
#include <ting/Exc.hpp>
#include <ting/types.hpp>
#include <ting/debug.hpp>
#include <ting/Buffer.hpp>
#include <ting/Array.hpp>
#include <ting/Ref.hpp>
#include <ting/Thread.hpp>
#include <ting/Ptr.hpp>

#include <list>

#include "Exc.hpp"



//#define M_ENABLE_AUMIKS_TRACE
#ifdef M_ENABLE_AUMIKS_TRACE
#define M_AUMIKS_TRACE(x) TRACE(<< "[aumiks] ") TRACE(x)
#else
#define M_AUMIKS_TRACE(x)
#endif



namespace aumiks{


//forward declarations
class Lib;
class Channel;



//TODO: add doxygen docs everywhere
class Effect : public ting::RefCounted{
	friend class aumiks::Channel;
public:
	/**
	 * @brief Called every time when the Channel is about to start playing.
	 * Called from separate thread.
	 * Default implementation does nothing. Override this method if needed.
     */
	virtual void Init_ts(){}
	
	/**
	 * @brief A result which should be returned by effect application method.
	 */
	enum E_Result{
		/**
		 * @brief Normal result after applying the effect.
		 * Return this if no any special actions needed after Effect applying.
		 * This value means that the effect does not require immediate sound stopping or
		 * continuation of channel playing after the sound is finished.
		 * So, returning this value indicates, that this effect does not affect the channel
		 * playing life time in any way.
		 */
		NORMAL,
		
		/**
		 * @brief Effect is still producing output.
		 * Return this if effect is producing output. If some of the effects added to the channel returns this value
		 * then the sound channel will be kept playing even if the original sound has finished.
		 * This is useful for such effects which produce some output even
		 * after the sound has ceased. For example and echo effect produces several echoes of the sound after the
		 * original sound has finished. This value is overridden by STOP_SOUND value, i.e. if some other effect added to this channel
		 * has returned STOP_SOUND then the channel playing will be stopped, no matter if this effect returns CONTINUE.
		 */
		CONTINUE,
		
		/**
		 * @brief Effect has request to stop the sound.
		 * Return this value if effect has fully suppressed the sound and there is no reason to
		 * continue sound playing since it will further result in silence due to this effect.
		 * For example the "fade out" effect may return this value when the sound is fully faded out.
		 * So, returning STOP_SOUND will result in immediate stopping of channel playing.
		 */
		STOP_SOUND
	};
	
	/**
	 * @brief Called when effect is to be applied to a portion of a playing sound.
     * @param buf - buffer containing portion of sound data.
	 * @param soundStopped - true if sound has finished playing, false otherwise.
     * @return One of the E_Result values. See E_Result description for more info.
     */
	virtual E_Result ApplyToSmpBuf11025Mono16(ting::Buffer<ting::s32>& buf, bool soundStopped){return NORMAL;}
	
	//TODO: add doxygen docs for each method
	virtual E_Result ApplyToSmpBuf11025Stereo16(ting::Buffer<ting::s32>& buf, bool soundStopped){return NORMAL;}
	virtual E_Result ApplyToSmpBuf22050Mono16(ting::Buffer<ting::s32>& buf, bool soundStopped){return NORMAL;}
	virtual E_Result ApplyToSmpBuf22050Stereo16(ting::Buffer<ting::s32>& buf, bool soundStopped){return NORMAL;}
	virtual E_Result ApplyToSmpBuf44100Mono16(ting::Buffer<ting::s32>& buf, bool soundStopped){return NORMAL;}
	virtual E_Result ApplyToSmpBuf44100Stereo16(ting::Buffer<ting::s32>& buf, bool soundStopped){return NORMAL;}
	
private:
	template <unsigned freq, unsigned chans> inline E_Result ApplyToSmpBufImpl(ting::Buffer<ting::s32>& buf, bool soundStopped);
};



//base channel class
class Channel : public ting::RefCounted{
	friend class aumiks::Lib;

	ting::Inited<volatile bool, false> isPlaying;
	
	bool soundStopped;//used to indicate that sound has finished playing, but effects are still playing.

	ting::Inited<volatile bool, false> stopFlag;//indicates that playing should stop immediately
	
	//list of effects
	typedef std::list<ting::Ref<aumiks::Effect> > T_EffectsList;
	typedef T_EffectsList::iterator T_EffectsIter;
	T_EffectsList effects;
	
	inline void InitEffects(){
		for(T_EffectsIter i = this->effects.begin(); i != this->effects.end(); ++i){
			(*i)->Init_ts();
		}
	}
	
	inline void RemoveEffect(const ting::Ref<Effect>& e){
		for(T_EffectsIter i = this->effects.begin(); i != this->effects.end(); ++i){
			if((*i) == e){
				this->effects.erase(i);
				break;
			}
		}
	}
	
	template <unsigned freq, unsigned chans> inline bool ApplyEffectsToSmpBuf(ting::Buffer<ting::s32>& buf){
		bool ret = this->soundStopped;
		bool stopRequested = false;

		for(T_EffectsIter i = this->effects.begin(); i != this->effects.end(); ++i){
			switch((*i)->ApplyToSmpBufImpl<freq, chans>(buf, this->soundStopped)){
				case Effect::NORMAL:
					break;
				case Effect::CONTINUE:
					ret = false; //at least one of the effects has requested to continue the channel playing
					break;
				case Effect::STOP_SOUND:
					//Set the stop requested flag.
					//It is still necessary to apply remaining effects, since current filled sample buffer
					//will still be played, no matter that the channel playing will be stopped.
					//This is why we use flag instead of immediately returning true here, because the for-loop
					//needs to be finished.
					stopRequested = true;
					break;
				default:
					ASSERT(false)
					break;
			}
		}
		return ret || stopRequested;
	}

	
protected:
	Channel(){}
	
public:

	inline bool IsPlaying()const{
		return this->isPlaying;
	}

	inline void Play();

	inline void Stop(){
		this->stopFlag = true;
	}
	
	inline void AddEffect_ts(const ting::Ref<aumiks::Effect>& effect);
	
	inline void RemoveEffect_ts(const ting::Ref<aumiks::Effect>& effect);
	
protected:
	/**
	 * @brief Called when channel has been added to pool of playing channels.
	 */
	virtual void OnStart(){}
	
	/**
	 * @brief Called when channel has been removed from pool of playing channels.
	 */
	virtual void OnStop(){}
private:
	//this function is called by SoundThread when it needs more data to play.
	//return true to remove channel from playing channels list
	virtual bool FillSmpBuf11025Mono16(ting::Buffer<ting::s32>& buf){return true;}
	virtual bool FillSmpBuf11025Stereo16(ting::Buffer<ting::s32>& buf){return true;}
	virtual bool FillSmpBuf22050Mono16(ting::Buffer<ting::s32>& buf){return true;}
	virtual bool FillSmpBuf22050Stereo16(ting::Buffer<ting::s32>& buf){return true;}
	virtual bool FillSmpBuf44100Mono16(ting::Buffer<ting::s32>& buf){return true;}
	virtual bool FillSmpBuf44100Stereo16(ting::Buffer<ting::s32>& buf){return true;}
};



enum E_Format{
	MONO_16_11025,
	MONO_16_22050,
	MONO_16_44100,
	STEREO_16_11025,
	STEREO_16_22050,
	STEREO_16_44100
};



unsigned BytesPerFrame(E_Format format);



unsigned SamplesPerFrame(E_Format format);



class Lib : public ting::Singleton<Lib>{
	friend class aumiks::Channel;
	
	void AddEffectToChannel_ts(const ting::Ref<Channel>& ch, const ting::Ref<aumiks::Effect>& eff);
	
	void RemoveEffectFromChannel_ts(const ting::Ref<Channel>& ch, const ting::Ref<aumiks::Effect>& eff);
	
	void PlayChannel_ts(const ting::Ref<Channel>& ch);
	
	static inline void RemoveEffectFromChannelSync(const ting::Ref<Channel>& ch, const ting::Ref<aumiks::Effect>& eff){
		ch->RemoveEffect(eff);
	}
	
	static inline void AddEffectToChannelSync(const ting::Ref<Channel>& ch, const ting::Ref<aumiks::Effect>& eff){
		ch->effects.push_back(eff);
	}
public:
	/**
	 * @brief Create sound library singleton instance.
	 * Creates singleton instance of sound library object and
	 * opens sound device.
	 * @param bufferSizeMillis - size of desired playing buffer in milliseconds.
	 * @param format - format of sound output.
	 */
	Lib(ting::u16 bufferSizeMillis = 100, aumiks::E_Format format = STEREO_16_22050);
	
	inline void SetMuted(bool muted){
		this->mixerBuffer->isMuted = muted;
	}

	inline void SetUnmuted(bool unmuted){
		this->SetMuted(!unmuted);
	}

	inline void Mute(){
		this->SetMuted(true);
	}

	inline void Unmute(){
		this->SetMuted(false);
	}

	inline bool IsMuted()const{
		return this->mixerBuffer->isMuted;
	}

private:
	//Base class for mixer buffers of different formats
	class MixerBuffer{
		friend class aumiks::Lib;
	protected:
		MixerBuffer(size_t mixBufSizeInSamples) :
				mixBuf(mixBufSizeInSamples),
				smpBuf(mixBufSizeInSamples)
		{}

		template <unsigned freq, unsigned chans> inline bool FillSmpBufImpl(const ting::Ref<aumiks::Channel>& ch);
		
		ting::Array<ting::s32> mixBuf;
		ting::Array<ting::s32> smpBuf;

		ting::Inited<volatile bool, false> isMuted;
	public:
		virtual ~MixerBuffer(){}
		
	private:
		inline void CleanMixBuf(){
			memset(this->mixBuf.Begin(), 0, this->mixBuf.SizeInBytes());
		}
		
		//return true if channel has finished playing and should be removed from playing channels pool
		bool MixToMixBuf(const ting::Ref<aumiks::Channel>& ch);

		void CopyToPlayBuf(ting::Buffer<ting::u8>& playBuf);
		
		virtual bool FillSmpBuf(const ting::Ref<aumiks::Channel>& ch) = 0;
		
		virtual bool ApplyEffectsToSmpBuf(const ting::Ref<aumiks::Channel>& ch) = 0;
		
		void MixSmpBufToMixBuf();
		
		template <unsigned freq, unsigned chans> inline bool ApplyEffectsToSmpBufImpl(const ting::Ref<aumiks::Channel>& ch){
			return ch->ApplyEffectsToSmpBuf<freq, chans>(this->smpBuf);
		}
	};

	template <unsigned freq, unsigned chans> class MixerBufferImpl : public Lib::MixerBuffer{
		MixerBufferImpl(unsigned bufferSizeInSamples) :
				MixerBuffer(bufferSizeInSamples)
		{}

		//override
		virtual bool FillSmpBuf(const ting::Ref<aumiks::Channel>& ch){
			return this->FillSmpBufImpl<freq, chans>(ch);
		}

		//override
		virtual bool ApplyEffectsToSmpBuf(const ting::Ref<aumiks::Channel>& ch){
			return this->ApplyEffectsToSmpBufImpl<freq, chans>(ch);
		}

	public:
		inline static ting::Ptr<MixerBufferImpl> New(unsigned bufferSizeInSamples){
			return ting::Ptr<MixerBufferImpl>(
					new MixerBufferImpl(bufferSizeInSamples)
				);
		}
	};


public:	
	//base class for audio backends
	class AudioBackend{
	protected:
		inline void FillPlayBuf_ts(ting::Buffer<ting::u8>& playBuf){
			aumiks::Lib::Inst().FillPlayBuf_ts(playBuf);
		}
		
		inline AudioBackend(){}
	public:
		virtual ~AudioBackend(){}
	};
private:
	void FillPlayBuf_ts(ting::Buffer<ting::u8>& playBuf);
	
	ting::Mutex chPoolMutex;

	typedef std::list<ting::Ref<aumiks::Channel> > T_ChPool;
	typedef T_ChPool::iterator T_ChIter;
	T_ChPool chPool;

	T_ChPool chPoolToAdd;

	typedef std::pair<ting::Ref<aumiks::Channel>, ting::Ref<aumiks::Effect> > T_ChannelEffectPair;
	typedef std::list<T_ChannelEffectPair> T_ChannelEffectPairsList;
	typedef T_ChannelEffectPairsList::iterator T_ChannelEffectPairsIter;
	T_ChannelEffectPairsList effectsToAdd;
	T_ChannelEffectPairsList effectsToRemove;
	
	const ting::Ptr<MixerBuffer> mixerBuffer;
	
	static ting::Ptr<MixerBuffer> CreateMixerBuffer(unsigned bufferSizeMillis, E_Format format);

	//backend must be initialized after all the essential parts of aumiks are initialized,
	//because after the backend object is created, it starts calling the FillPlayBuf_ts() method periodically.
	const ting::Ptr<AudioBackend> audioBackend;
};



template <> inline bool Lib::MixerBuffer::FillSmpBufImpl<11025, 1>(const ting::Ref<aumiks::Channel>& ch){
	return ch->FillSmpBuf11025Mono16(this->smpBuf);
}
template <> inline bool Lib::MixerBuffer::FillSmpBufImpl<11025, 2>(const ting::Ref<aumiks::Channel>& ch){
	return ch->FillSmpBuf11025Stereo16(this->smpBuf);
}
template <> inline bool Lib::MixerBuffer::FillSmpBufImpl<22050, 1>(const ting::Ref<aumiks::Channel>& ch){
	return ch->FillSmpBuf22050Mono16(this->smpBuf);
}
template <> inline bool Lib::MixerBuffer::FillSmpBufImpl<22050, 2>(const ting::Ref<aumiks::Channel>& ch){
	return ch->FillSmpBuf22050Stereo16(this->smpBuf);
}
template <> inline bool Lib::MixerBuffer::FillSmpBufImpl<44100, 1>(const ting::Ref<aumiks::Channel>& ch){
	return ch->FillSmpBuf44100Mono16(this->smpBuf);
}
template <> inline bool Lib::MixerBuffer::FillSmpBufImpl<44100, 2>(const ting::Ref<aumiks::Channel>& ch){
	return ch->FillSmpBuf44100Stereo16(this->smpBuf);
}


template <> inline Effect::E_Result Effect::ApplyToSmpBufImpl<11025, 1>(ting::Buffer<ting::s32>& buf, bool soundStopped){
	return this->ApplyToSmpBuf11025Mono16(buf, soundStopped);
}
template <> inline Effect::E_Result Effect::ApplyToSmpBufImpl<11025, 2>(ting::Buffer<ting::s32>& buf, bool soundStopped){
	return this->ApplyToSmpBuf11025Stereo16(buf, soundStopped);
}
template <> inline Effect::E_Result Effect::ApplyToSmpBufImpl<22050, 1>(ting::Buffer<ting::s32>& buf, bool soundStopped){
	return this->ApplyToSmpBuf22050Mono16(buf, soundStopped);
}
template <> inline Effect::E_Result Effect::ApplyToSmpBufImpl<22050, 2>(ting::Buffer<ting::s32>& buf, bool soundStopped){
	return this->ApplyToSmpBuf22050Stereo16(buf, soundStopped);
}
template <> inline Effect::E_Result Effect::ApplyToSmpBufImpl<44100, 1>(ting::Buffer<ting::s32>& buf, bool soundStopped){
	return this->ApplyToSmpBuf44100Mono16(buf, soundStopped);
}
template <> inline Effect::E_Result Effect::ApplyToSmpBufImpl<44100, 2>(ting::Buffer<ting::s32>& buf, bool soundStopped){
	return this->ApplyToSmpBuf44100Stereo16(buf, soundStopped);
}
		


//base class for all sounds
class Sound : public ting::RefCounted{
protected:
	Sound(){}
public:
	virtual ting::Ref<aumiks::Channel> CreateChannel()const = 0;
};



inline void Channel::Play(){
	aumiks::Lib::Inst().PlayChannel_ts(ting::Ref<aumiks::Channel>(this));
}



inline void Channel::AddEffect_ts(const ting::Ref<aumiks::Effect>& effect){
	aumiks::Lib::Inst().AddEffectToChannel_ts(
			ting::Ref<Channel>(this),
			effect
		);
}



inline void Channel::RemoveEffect_ts(const ting::Ref<aumiks::Effect>& effect){
	aumiks::Lib::Inst().RemoveEffectFromChannel_ts(
			ting::Ref<Channel>(this),
			effect
		);
}



} //~namespace
