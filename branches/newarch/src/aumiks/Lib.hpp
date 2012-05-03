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

/**
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once



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
	friend class aumiks::MixChannel;
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
	
	//size of the playing buffer in samples
	unsigned bufSizeInSamples;
	
	inline unsigned BufSizeInSamples(){
		return this->bufSizeInSamples;
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
