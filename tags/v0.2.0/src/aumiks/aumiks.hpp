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
class AudioBackend;



/**
 * @brief Base class for effect classes which can be applied to a playing sound.
 * The effects should derive from this class and re-implement the virtual methods
 * which are called to apply the effect to a sound when it is played.
 * The effects can be added to the playing channel.
 */
class Effect : public ting::RefCounted{
	friend class aumiks::Channel;
	friend class aumiks::Lib;

	typedef std::list<ting::Ref<aumiks::Effect> > T_EffectsList;
	typedef T_EffectsList::iterator T_EffectsIter;

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
	 * Depending on the output sound format the corresponding method is called.
	 * Note, that when effect is used as a global effect, then the return value from
	 * this method is ignored. It only matters when effect is added to channel.
	 * @param buf - buffer containing portion of sound data.
	 * @param soundStopped - true if sound has finished playing, false otherwise.
	 * @return One of the E_Result values. See E_Result description for more info.
	 */
	virtual E_Result ApplyToBuf11025Mono16(ting::Buffer<ting::s32>& buf, bool soundStopped){
		return NORMAL;
	}

	/**
	 * @brief Called when effect is to be applied to a portion of a playing sound.
	 * See description of Effect::ApplyToSmpBuf11025Mono16() method.
	 */
	virtual E_Result ApplyToBuf11025Stereo16(ting::Buffer<ting::s32>& buf, bool soundStopped){
		return NORMAL;
	}

	/**
	 * @brief Called when effect is to be applied to a portion of a playing sound.
	 * See description of Effect::ApplyToSmpBuf11025Mono16() method.
	 */
	virtual E_Result ApplyToBuf22050Mono16(ting::Buffer<ting::s32>& buf, bool soundStopped){
		return NORMAL;
	}

	/**
	 * @brief Called when effect is to be applied to a portion of a playing sound.
	 * See description of Effect::ApplyToSmpBuf11025Mono16() method.
	 */
	virtual E_Result ApplyToBuf22050Stereo16(ting::Buffer<ting::s32>& buf, bool soundStopped){
		return NORMAL;
	}

	/**
	 * @brief Called when effect is to be applied to a portion of a playing sound.
	 * See description of Effect::ApplyToSmpBuf11025Mono16() method.
	 */
	virtual E_Result ApplyToBuf44100Mono16(ting::Buffer<ting::s32>& buf, bool soundStopped){
		return NORMAL;
	}

	/**
	 * @brief Called when effect is to be applied to a portion of a playing sound.
	 * See description of Effect::ApplyToSmpBuf11025Mono16() method.
	 */
	virtual E_Result ApplyToBuf44100Stereo16(ting::Buffer<ting::s32>& buf, bool soundStopped){
		return NORMAL;
	}

private:
	template <unsigned freq, unsigned chans> inline E_Result ApplyToBufImpl(ting::Buffer<ting::s32>& buf, bool soundStopped);
};



/**
 * @brief Base class of a channel for playing the sound.
 * Usually, the effects are created by Sound class implementations using CreateChannel() method.
 */
class Channel : public ting::RefCounted{
	friend class aumiks::Lib;

	ting::Inited<volatile bool, false> isPlaying;

	bool soundStopped;//used to indicate that sound has finished playing, but effects are still playing.

	ting::Inited<volatile bool, false> stopFlag;//indicates that playing should stop immediately

private:
	Effect::T_EffectsList effects;

	inline void InitEffects(){
		for(Effect::T_EffectsIter i = this->effects.begin(); i != this->effects.end(); ++i){
			(*i)->Init_ts();
		}
	}

	inline void RemoveEffect(const ting::Ref<Effect>& e){
		for(Effect::T_EffectsIter i = this->effects.begin(); i != this->effects.end(); ++i){
			if((*i) == e){
				this->effects.erase(i);
				break;
			}
		}
	}

	template <unsigned freq, unsigned chans> inline bool ApplyEffectsToSmpBuf(ting::Buffer<ting::s32>& buf){
		bool ret = this->soundStopped;
		bool stopRequested = false;

		for(Effect::T_EffectsIter i = this->effects.begin(); i != this->effects.end(); ++i){
			switch((*i)->ApplyToBufImpl<freq, chans>(buf, this->soundStopped)){
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

	virtual ~Channel()throw(){}
	
	/**
	 * @brief Check if sound is currently playing.
	 * @return true if channel is playing.
	 * @return false otherwise.
	 */
	inline bool IsPlaying()const{
		return this->isPlaying;
	}

	/**
	 * @brief Start playing of this channel.
	 */
	inline void Play();

	/**
	 * @brief Stop playing of this channel.
	 */
	inline void Stop(){
		this->stopFlag = true;
	}

	/**
	 * @brief Add effect to the channel.
	 * It is allowed to add effects during channel playing.
	 * The single effect instance can only be added to one channel. Adding single
	 * Effect instance to more than one channel will result in undefined behavior.
	 * @param effect - the effect to add.
	 */
	inline void AddEffect_ts(const ting::Ref<aumiks::Effect>& effect);

	/**
	 * @brief Remove effect from the channel.
	 * It is allowed to remove effects during channel playing.
	 * @param effect - effect to remove.
	 */
	inline void RemoveEffect_ts(const ting::Ref<aumiks::Effect>& effect);

	/**
	 * @brief Remove all effects from channel.
	 */
	inline void RemoveAllEffects_ts();
protected:
	/**
	 * @brief Called when channel has been added to pool of playing channels.
	 */
	virtual void OnStart(){}

	/**
	 * @brief Called when channel has been removed from pool of playing channels.
	 */
	virtual void OnStop(){}

	/**
	 * @brief This function is called when more data to play is needed.
	 * Override this method in your Channel implementation.
	 * Depending on the selected output format (sampling rate, mono/stereo) the corresponding method is called.
	 * The return value indicates whether the sound has finished playing or not.
	 * Note, that channel playing may continue even if sound has stopped playing, this is
	 * possible if there are any effects added to this channel which keeps playing, for example
	 * an echo effect.
	 * @param buf - the sample buffer to fill with the data to play.
	 * @return true if sound playing has finished.
	 * @return false otherwise.
	 */
	virtual bool FillSmpBuf11025Mono16(ting::Buffer<ting::s32>& buf){
		return true;
	}

	/**
	 * @brief This function is called when more data to play is needed.
	 * See description of Channel::FillSmpBuf11025Mono16() method.
	 */
	virtual bool FillSmpBuf11025Stereo16(ting::Buffer<ting::s32>& buf){
		return true;
	}

	/**
	 * @brief This function is called when more data to play is needed.
	 * See description of Channel::FillSmpBuf11025Mono16() method.
	 */
	virtual bool FillSmpBuf22050Mono16(ting::Buffer<ting::s32>& buf){
		return true;
	}

	/**
	 * @brief This function is called when more data to play is needed.
	 * See description of Channel::FillSmpBuf11025Mono16() method.
	 */
	virtual bool FillSmpBuf22050Stereo16(ting::Buffer<ting::s32>& buf){
		return true;
	}

	/**
	 * @brief This function is called when more data to play is needed.
	 * See description of Channel::FillSmpBuf11025Mono16() method.
	 */
	virtual bool FillSmpBuf44100Mono16(ting::Buffer<ting::s32>& buf){
		return true;
	}

	/**
	 * @brief This function is called when more data to play is needed.
	 * See description of Channel::FillSmpBuf11025Mono16() method.
	 */
	virtual bool FillSmpBuf44100Stereo16(ting::Buffer<ting::s32>& buf){
		return true;
	}
};



/**
 * @brief Sound output format.
 * Enumeration defining possible sound output formats.
 * All currently supported formats are 16 bits per sample.
 */
enum E_Format{
	/**
	 * @brief Mono 11025 Hz.
	 */
	MONO_16_11025,

	/**
	 * @brief Mono 22050 Hz.
	 */
	MONO_16_22050,

	/**
	 * @brief Mono 44100 Hz.
	 */
	MONO_16_44100,

	/**
	 * @brief Stereo 11025 Hz.
	 */
	STEREO_16_11025,

	/**
	 * @brief Stereo 22050 Hz.
	 */
	STEREO_16_22050,

	/**
	 * @brief Stereo 44100 Hz.
	 */
	STEREO_16_44100
};



/**
 * @brief Returns frame size for given sound output format.
 * Returns number of bytes per frame for given sound output format.
 * The sound frame is the sound data for a single sampling rate tick.
 * Each frame consists of number of channels samples (e.g. mono: 1 frame = 1 sample, stereo: 1 frame = 2 samples).
 * @param format - format to return the frame size of.
 * @return the size of the frame in bytes.
 */
unsigned BytesPerFrame(E_Format format);



/**
 * @brief Returns number of samples per frame for given sound output format.
 * In fact, this function returns number of channels of the given sound output format.
 * @param format - format for which to return the frame size in samples.
 * @return size of the frame in samples.
 */
unsigned SamplesPerFrame(E_Format format);



/**
 * @brief Returns sampling rate for given sound output format.
 * @return Sampling rate in Hz.
 */
unsigned SamplingRate(E_Format format);



/**
 * @brief aumiks library singleton class.
 * This is a main class of the aumiks library.
 * Before using the library one has to create a single instance of the Lib class.
 * It will perform necessary sound output initializations and open sound output device.
 * Destroying the object will close the sound output device and clean all the resources.
 */
class Lib : public ting::IntrusiveSingleton<Lib>{
	friend class ting::IntrusiveSingleton<Lib>;
	static ting::IntrusiveSingleton<Lib>::T_Instance instance;
	
	friend class aumiks::Channel;
	friend class aumiks::AudioBackend;

	void AddEffectToChannel_ts(const ting::Ref<Channel>& ch, const ting::Ref<aumiks::Effect>& eff);

	void RemoveEffectFromChannel_ts(const ting::Ref<Channel>& ch, const ting::Ref<aumiks::Effect>& eff);

	void RemoveAllEffectsFromChannel_ts(const ting::Ref<Channel>& ch);

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
	 * @param bufferSizeMillis - size of desired playing buffer in milliseconds. Use smaller buffers for higher latency.
	 *                           Note, that very small buffer may result in bigger overhead and lags. The same applies to very big buffer sizes.
	 * @param format - format of sound output.
	 */
	Lib(ting::u16 bufferSizeMillis = 100, aumiks::E_Format format = STEREO_16_22050);

	/**
	 * @brief Mute sound output.
	 * Mute the sound output. This is the same as the resulting sound volume would be set to zero.
	 * I.e. all the computational resources for sound mixing etc. are still being consumed when sound is muted.
	 * @param muted - pass true to mute the sound, false to un-mute.
	 */
	inline void SetMuted(bool muted){
		this->mixerBuffer->isMuted = muted;
	}

	/**
	 * @brief Unmute the sound.
	 * Inversion of Lib::SetMuted() method.
	 * See description of Lib::SetMuted() method for more info.
	 * @param unmuted - true to un-mute the sound, false to mute.
	 */
	inline void SetUnmuted(bool unmuted){
		this->SetMuted(!unmuted);
	}

	/**
	 * @brief Mute the sound.
	 * See description of Lib::SetMuted() method for more info.
	 */
	inline void Mute(){
		this->SetMuted(true);
	}

	/**
	 * @brief Un-mute the sound.
	 * See description of Lib::SetMuted() method for more info.
	 */
	inline void Unmute(){
		this->SetMuted(false);
	}

	/**
	 * @brief Check if sound output is muted or not.
	 * @return true if sound output is muted.
	 * @return false otherwise.
	 */
	inline bool IsMuted()const{
		return this->mixerBuffer->isMuted;
	}

	/**
	 * @brief Sets the paused state of the audio engine.
	 * Moves engine to paused or resumed state depending on the passed parameter value.
	 * In paused state the engine still holds the audio device open but
	 * does not play the main audio buffer, thus does not consume CPU resources.
	 * The method is not thread-safe and should be called from the thread where Lib object was created.
     * @param pause - determines whether to pause or resume the audio engine. Pass true to pause and false to resume.
     */
	inline void SetPaused(bool pause);	
	/**
	 * @brief Pause audio engine.
	 * Moves the audio engine to paused state.
	 * Essentially it just calls the SetPaused_ts(true) method.
	 * The method is not thread-safe and should be called from the thread where Lib object was created.
     */
	inline void Pause(){
		this->SetPaused(true);
	}
	
	/**
	 * @brief Resume audio engine.
	 * Un-pauses the audio engine. See Pause_ts() method for more info.
	 * Essentially it just calls the SetPaused_ts(false) method.
	 * The method is not thread-safe and should be called from the thread where Lib object was created.
     */
	inline void Resume(){
		this->SetPaused(false);
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

		virtual void ApplyEffectsToMixBuf() = 0;

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

		//override
		virtual void ApplyEffectsToMixBuf(){
			aumiks::Lib::Inst().ApplyEffectsToMixBuf<freq, chans>(this->mixBuf);
		}
	public:
		inline static ting::Ptr<MixerBufferImpl> New(unsigned bufferSizeInSamples){
			return ting::Ptr<MixerBufferImpl>(
					new MixerBufferImpl(bufferSizeInSamples)
				);
		}
	};

private:
	//this function is not thread-safe, but it is supposed to be called from special audio thread
	void FillPlayBuf(ting::Buffer<ting::u8>& playBuf);

	ting::Mutex mutex;

	typedef std::list<ting::Ref<aumiks::Channel> > T_ChannelList;
	typedef T_ChannelList::iterator T_ChannelIter;
	T_ChannelList chPool;

	T_ChannelList channelsToAdd;

	Effect::T_EffectsList effects;//list of effects applied to final mixing buffer

	template <unsigned freq, unsigned chans> inline void ApplyEffectsToMixBuf(ting::Buffer<ting::s32>& mixBuf){
		for(Effect::T_EffectsIter i = this->effects.begin(); i != this->effects.end(); ++i){
			(*i)->ApplyToBufImpl<freq, chans>(mixBuf, false);
		}
	}

	Effect::T_EffectsList effectsToAdd;
	Effect::T_EffectsList effectsToRemove;

	ting::Inited<bool, false> clearEffects;//flag indicating that removing of all effects requested

public:
	/**
	 * @brief Add global effect.
	 * Adds the effect to the list of global effects which are applied to the
	 * final mixing buffer after all the playing channels are mixed.
	 * @param effect - effect to add.
	 */
	inline void AddEffect_ts(const ting::Ref<Effect>& effect){
		ASSERT(effect.IsValid())

		{
			ting::Mutex::Guard mutexGuard(this->mutex);

			this->effectsToAdd.push_back(effect);
		}
	}

	/**
	 * @brief Remove global effect.
	 * Removes the effect from the list of global effects which are applied to the
	 * final mixing buffer after all the playing channels are mixed.
	 * @param effect - effect to remove.
	 */
	inline void RemoveEffect_ts(const ting::Ref<Effect>& effect){
		ASSERT(effect.IsValid())

		{
			ting::Mutex::Guard mutexGuard(this->mutex);

			this->effectsToRemove.push_back(effect);
		}
	}

	inline void RemoveAllEffects_ts(){
		ting::Mutex::Guard mutexGuard(this->mutex);

		this->clearEffects = true;
	}
private:
	typedef std::pair<ting::Ref<aumiks::Channel>, ting::Ref<aumiks::Effect> > T_ChannelEffectPair;
	typedef std::list<T_ChannelEffectPair> T_ChannelEffectPairsList;
	typedef T_ChannelEffectPairsList::iterator T_ChannelEffectPairsIter;
	T_ChannelEffectPairsList effectsToAddToChan;
	T_ChannelEffectPairsList effectsToRemoveFromChan;

	//list of channels from which it is requested to remove all effects
	T_ChannelList effectsToClear;

	const ting::Ptr<MixerBuffer> mixerBuffer;

	static ting::Ptr<MixerBuffer> CreateMixerBuffer(unsigned bufferSizeMillis, E_Format format);

	//backend must be initialized after all the essential parts of aumiks are initialized,
	//because after the backend object is created, it starts calling the FillPlayBuf() method periodically.
	const ting::Ptr<AudioBackend> audioBackend;
};



//base class for audio backends
//TODO: make it private somehow
class AudioBackend{
protected:
	inline void FillPlayBuf(ting::Buffer<ting::u8>& playBuf){
		aumiks::Lib::Inst().FillPlayBuf(playBuf);
	}

	inline AudioBackend(){}
	
public:
	virtual ~AudioBackend()throw(){}
	
	virtual void SetPaused(bool pause){}
};



inline void Lib::SetPaused(bool pause){
	ASSERT(this->audioBackend)
	this->audioBackend->SetPaused(pause);
}



//Full template specializations
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



//Full template specializations
template <> inline Effect::E_Result Effect::ApplyToBufImpl<11025, 1>(ting::Buffer<ting::s32>& buf, bool soundStopped){
	return this->ApplyToBuf11025Mono16(buf, soundStopped);
}
template <> inline Effect::E_Result Effect::ApplyToBufImpl<11025, 2>(ting::Buffer<ting::s32>& buf, bool soundStopped){
	return this->ApplyToBuf11025Stereo16(buf, soundStopped);
}
template <> inline Effect::E_Result Effect::ApplyToBufImpl<22050, 1>(ting::Buffer<ting::s32>& buf, bool soundStopped){
	return this->ApplyToBuf22050Mono16(buf, soundStopped);
}
template <> inline Effect::E_Result Effect::ApplyToBufImpl<22050, 2>(ting::Buffer<ting::s32>& buf, bool soundStopped){
	return this->ApplyToBuf22050Stereo16(buf, soundStopped);
}
template <> inline Effect::E_Result Effect::ApplyToBufImpl<44100, 1>(ting::Buffer<ting::s32>& buf, bool soundStopped){
	return this->ApplyToBuf44100Mono16(buf, soundStopped);
}
template <> inline Effect::E_Result Effect::ApplyToBufImpl<44100, 2>(ting::Buffer<ting::s32>& buf, bool soundStopped){
	return this->ApplyToBuf44100Stereo16(buf, soundStopped);
}



/**
 * @brief Base class for sounds.
 * A sound object is an object which holds all the initial data required to play a particular sound.
 * Sound object is normally used to create an instances of a channel to play that sound.
 */
class Sound : public ting::RefCounted{
protected:
	Sound(){}
public:

	/**
	 * @brief Channel factory method.
	 * Creates an instance of the channel which can later be used to play that sound.
	 * @return a newly created instance of the channel for this sound.
	 */
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



inline void Channel::RemoveAllEffects_ts(){
	aumiks::Lib::Inst().RemoveAllEffectsFromChannel_ts(
			ting::Ref<Channel>(this)
		);
}



} //~namespace
