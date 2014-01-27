/* The MIT License:

Copyright (c) 2012-2013 Ivan Gagis

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

#include "Effect.hpp"



namespace aumiks{



class Lib;
class MixChannel;



/**
 * @brief Base class of a channel for playing the sound.
 * Usually, channels are created by Sound class implementations using CreateChannel() method.
 */
class Channel : public SampleBufferFiller, public virtual ting::RefCounted{
	friend class aumiks::Lib;
	friend class aumiks::MixChannel;
	
	ting::Inited<volatile bool, false> stoppedFlag;
	
	aumiks::SampleBufferFiller* lastFillerInChain;
	
private:
	Effect::T_EffectsList effects;

	inline bool FillSmpBufAndApplyEffects(ting::Buffer<ting::s32>& buf){
		if(this->stoppedFlag){
			return true;
		}
		
		ASSERT(this->lastFillerInChain)

		return this->lastFillerInChain->FillSmpBufInternal(buf);
	}
	
protected:
	//TODO: doxygen
	ting::Inited<volatile bool, false> stopFlag;
	
	Channel() :
			lastFillerInChain(this)
	{}

public:

	virtual ~Channel()throw(){}

	/**
	 * @brief Start playing of this channel.
	 */
	void Play_ts();

	/**
	 * @brief Requests channel to stop playing.
	 * This is a request to stop playing the channel, the channel may stop not
	 * immediately, depending on implementation of the particular Channel.
	 * Eventually, channel normally moves to stopped state, but it may depend on
	 * the implementation of a particular channel. For example, the sound may stop
	 * by quickly fading out to avoid clicks and gaps.
	 * Once stopped, the channel cannot be started again, at least trying to
	 * will result in undefined behavior.
	 * Instead, one needs to create a new channel.
	 */
	inline void Stop_ts()throw(){
		this->stopFlag = true;
	}
	
	/**
	 * @brief Stop playing the channel.
	 * Stops playing the channel immediately. After calling this method the
	 * channel is moved to stopped state.
	 * Once stopped, the channel cannot be started again, at least trying to
	 * will result in undefined behavior.
	 * Instead, one needs to create a new channel.
     */
	inline void StopNow_ts()throw(){
		this->stoppedFlag = true;
	}

	/**
	 * @brief Tells if channel has finished playing.
	 * Even if channel is paused it is considered as not stopped.
	 * Right after creation the channel is not stopped. Once stopped the channel
	 * cannot be started again. One needs to create a new channel instead.
     * @return true if channel has stopped playing.
	 * @return false otherwise.
     */
	inline bool IsStopped_ts()throw(){
		return this->stoppedFlag;
	}

	/**
	 * @brief Add effect to the channel.
	 * It is allowed to add effects during channel playing.
	 * The single effect instance can only be added to one channel. Adding single
	 * Effect instance to more than one channel will result in undefined behavior.
	 * @param effect - the effect to add.
	 */
	void AddEffect_ts(const ting::Ref<aumiks::Effect>& effect);

	/**
	 * @brief Remove effect from the channel.
	 * It is allowed to remove effects during channel playing.
	 * It only sends a request to the audio thread to remove the effect. The audio
	 * thread will remove the effect as soon as possible. So, it does not mean that the
	 * effect is removed immediately.
	 * @param effect - effect to remove.
	 */
	void RemoveEffect_ts(const ting::Ref<aumiks::Effect>& effect);

	/**
	 * @brief Remove all effects from channel.
	 * It only sends a request to the audio thread to remove effects. The audio
	 * thread will remove effects as soon as possible. So, it does not mean that
	 * effects are removed immediately.
	 */
	void RemoveAllEffects_ts();
};

}//~namespace
