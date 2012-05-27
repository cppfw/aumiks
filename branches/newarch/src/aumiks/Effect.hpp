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

#include <ting/Ref.hpp>



namespace aumiks{



class Channel;



/**
 * @brief Interface for filling the sample buffer.
 * TODO: write docs about chain of sample buffer fillers.
 */
class SampleBufferFiller{
	//pointer to the next buffer filler in the chain.
	//If 0 then this is the last one in the chain.
	ting::Inited<SampleBufferFiller*, 0> next;
	
protected:
    //TODO: re-wise docs
	/**
	 * @brief This function is called when more data to play is needed.
	 * Override this method in your Channel implementation.
	 * Depending on the selected output format (sampling rate, mono/stereo) the corresponding method is called.
	 * The return value indicates whether the sound has finished playing or not.
	 * Note, that channel playing may continue even if sound has stopped playing, this is
	 * possible if there are any effects added to this channel which keeps playing, for example
	 * an echo effect.
	 * @param buf - the sample buffer to fill with the data to play.
	 * @param freq - sampling rate in Hertz.
	 * @param chans - number of channels (1 = mono, 2 = stereo, etc.).
	 * @return true if sound playing has finished.
	 * @return false otherwise.
	 */
	virtual bool FillSmpBuf(ting::Buffer<ting::s32>& buf, unsigned freq, unsigned chans) = 0;
	
	//TODO: doxygen
	inline bool FillSmpBufFromNextByChain(ting::Buffer<ting::s32>& buf, unsigned freq, unsigned chans){
		if(this->next == 0){
			return true;
		}
		return this->next->FillSmpBuf(buf, freq, chans);
	}
	
public:
	virtual ~SampleBufferFiller()throw(){}
};



/**
 * @brief Base class for effect classes which can be applied to a playing sound.
 * The effects should derive from this class and re-implement the virtual methods
 * which are called to apply the effect to a sound when it is played.
 * The effects can be added to the playing channel.
 */
class Effect : public SampleBufferFiller, public virtual ting::RefCounted{
	friend class aumiks::Channel;
	
	typedef std::list<ting::Ref<aumiks::Effect> > T_EffectsList;
	typedef T_EffectsList::iterator T_EffectsIter;

public:

	/**
	 * @brief Called every time when the Channel is about to start playing.
	 * Called from separate thread, should be thread safe.
	 * Default implementation does nothing. Override this method if needed.
	 */
	virtual void Init_ts(){}

	
	//TODO:
//	/**
//	 * @brief A result which should be returned by effect application method.
//	 */
//	enum E_Result{
//		/**
//		 * @brief Normal result after applying the effect.
//		 * Return this if no any special actions needed after Effect applying.
//		 * This value means that the effect does not require immediate sound stopping or
//		 * continuation of channel playing after the sound is finished.
//		 * So, returning this value indicates, that this effect does not affect the channel
//		 * playing life time in any way.
//		 */
//		NORMAL,
//
//		/**
//		 * @brief Effect is still producing output.
//		 * Return this if effect is producing output. If some of the effects added to the channel returns this value
//		 * then the sound channel will be kept playing even if the original sound has finished.
//		 * This is useful for such effects which produce some output even
//		 * after the sound has ceased. For example and echo effect produces several echoes of the sound after the
//		 * original sound has finished. This value is overridden by STOP_SOUND value, i.e. if some other effect added to this channel
//		 * has returned STOP_SOUND then the channel playing will be stopped, no matter if this effect returns CONTINUE.
//		 */
//		CONTINUE,
//
//		/**
//		 * @brief Effect has request to stop the sound.
//		 * Return this value if effect has fully suppressed the sound and there is no reason to
//		 * continue sound playing since it will further result in silence due to this effect.
//		 * For example the "fade out" effect may return this value when the sound is fully faded out.
//		 * So, returning STOP_SOUND will result in immediate stopping of channel playing.
//		 */
//		STOP_SOUND
//	};

	//TODO:
//	/**
//	 * @brief Called when effect is to be applied to a portion of a playing sound.
//	 * Depending on the output sound format the corresponding method is called.
//	 * Note, that when effect is used as a global effect, then the return value from
//	 * this method is ignored. It only matters when effect is added to channel.
//	 * @param buf - buffer containing portion of sound data.
//	 * @param soundStopped - true if sound has finished playing, false otherwise.
//	 * @return One of the E_Result values. See E_Result description for more info.
//	 */
//	virtual E_Result ApplyToBuf11025Mono16(ting::Buffer<ting::s32>& buf, bool soundStopped){
//		return NORMAL;
//	}
//
//	/**
//	 * @brief Called when effect is to be applied to a portion of a playing sound.
//	 * See description of Effect::ApplyToSmpBuf11025Mono16() method.
//	 */
//	virtual E_Result ApplyToBuf11025Stereo16(ting::Buffer<ting::s32>& buf, bool soundStopped){
//		return NORMAL;
//	}
//
//	/**
//	 * @brief Called when effect is to be applied to a portion of a playing sound.
//	 * See description of Effect::ApplyToSmpBuf11025Mono16() method.
//	 */
//	virtual E_Result ApplyToBuf22050Mono16(ting::Buffer<ting::s32>& buf, bool soundStopped){
//		return NORMAL;
//	}
//
//	/**
//	 * @brief Called when effect is to be applied to a portion of a playing sound.
//	 * See description of Effect::ApplyToSmpBuf11025Mono16() method.
//	 */
//	virtual E_Result ApplyToBuf22050Stereo16(ting::Buffer<ting::s32>& buf, bool soundStopped){
//		return NORMAL;
//	}
//
//	/**
//	 * @brief Called when effect is to be applied to a portion of a playing sound.
//	 * See description of Effect::ApplyToSmpBuf11025Mono16() method.
//	 */
//	virtual E_Result ApplyToBuf44100Mono16(ting::Buffer<ting::s32>& buf, bool soundStopped){
//		return NORMAL;
//	}
//
//	/**
//	 * @brief Called when effect is to be applied to a portion of a playing sound.
//	 * See description of Effect::ApplyToSmpBuf11025Mono16() method.
//	 */
//	virtual E_Result ApplyToBuf44100Stereo16(ting::Buffer<ting::s32>& buf, bool soundStopped){
//		return NORMAL;
//	}
//
//private:
//	template <unsigned freq, unsigned chans> inline E_Result ApplyToBufImpl(ting::Buffer<ting::s32>& buf, bool soundStopped);
};


}//~namespace
