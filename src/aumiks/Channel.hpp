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

#include "Effect.hpp"



namespace aumiks{



class Lib;
class MixChannel;



/**
 * @brief Base class of a channel for playing the sound.
 * Usually, the effects are created by Sound class implementations using CreateChannel() method.
 */
class Channel : public SampleBufferFiller, public virtual ting::RefCounted{
	friend class aumiks::Lib;
	friend class aumiks::MixChannel;
	
	ting::Inited<volatile bool, false> isPlaying;
	
	aumiks::SampleBufferFiller* lastFillerInChain;
	
private:
	Effect::T_EffectsList effects;

	inline void InitEffects()throw(){
		for(Effect::T_EffectsIter i = this->effects.begin(); i != this->effects.end(); ++i){
			(*i)->Init_ts();
		}
	}

protected:
	inline bool FillSmpBufAndApplyEffects(ting::Buffer<ting::s32>& buf, unsigned freq, unsigned chans){
		ASSERT(buf.Size() % chans == 0)
	
//		TRACE(<< "Channel::FillSmpBufAndApplyEffects(): isPlaying = " << this->isPlaying << std::endl)
		
		if(!this->isPlaying){
			return true;
		}

		ASSERT(this->lastFillerInChain)

		bool ret = this->lastFillerInChain->FillSmpBuf(buf, freq, chans);

		return ret;
	}
	
	Channel() :
			lastFillerInChain(this)
	{}

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
	void Play();

	/**
	 * @brief Stop playing of this channel.
	 */
	inline void Stop(){
		this->isPlaying = false;
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
	 * @param effect - effect to remove.
	 */
	void RemoveEffect_ts(const ting::Ref<aumiks::Effect>& effect);

	/**
	 * @brief Remove all effects from channel.
	 */
	void RemoveAllEffects_ts();
protected:
	/**
	 * @brief Called when channel has been added to pool of playing channels.
	 */
	virtual void OnStart()throw(){}

	/**
	 * @brief Called when channel has been removed from pool of playing channels.
	 */
	virtual void OnStop()throw(){}
};

}//~namespace
