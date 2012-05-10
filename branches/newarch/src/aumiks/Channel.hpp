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



/**
 * @brief Base class of a channel for playing the sound.
 * Usually, the effects are created by Sound class implementations using CreateChannel() method.
 */
class Channel : public virtual ting::RefCounted{
	friend class aumiks::Lib;

	ting::Inited<volatile bool, false> isPlaying;

	bool soundStopped;//used to indicate that sound has finished playing, but effects are still playing.

	ting::Inited<volatile bool, false> stopFlag;//indicates that playing should stop immediately

private:
	Effect::T_EffectsList effects;

	//TODO:
//	inline void InitEffects(){
//		for(Effect::T_EffectsIter i = this->effects.begin(); i != this->effects.end(); ++i){
//			(*i)->Init_ts();
//		}
//	}
//
//	inline void RemoveEffect(const ting::Ref<Effect>& e){
//		for(Effect::T_EffectsIter i = this->effects.begin(); i != this->effects.end(); ++i){
//			if((*i) == e){
//				this->effects.erase(i);
//				break;
//			}
//		}
//	}
//
//	template <unsigned freq, unsigned chans> inline bool ApplyEffectsToSmpBuf(ting::Buffer<ting::s32>& buf){
//		bool ret = this->soundStopped;
//		bool stopRequested = false;
//
//		for(Effect::T_EffectsIter i = this->effects.begin(); i != this->effects.end(); ++i){
//			switch((*i)->ApplyToBufImpl<freq, chans>(buf, this->soundStopped)){
//				case Effect::NORMAL:
//					break;
//				case Effect::CONTINUE:
//					ret = false; //at least one of the effects has requested to continue the channel playing
//					break;
//				case Effect::STOP_SOUND:
//					//Set the stop requested flag.
//					//It is still necessary to apply remaining effects, since current filled sample buffer
//					//will still be played, no matter that the channel playing will be stopped.
//					//This is why we use flag instead of immediately returning true here, because the for-loop
//					//needs to be finished.
//					stopRequested = true;
//					break;
//				default:
//					ASSERT(false)
//					break;
//			}
//		}
//		return ret || stopRequested;
//	}
	
protected:
	bool FillSmpBufAndApplyEffects(ting::Buffer<ting::s32>& buf, unsigned freq, unsigned chans);
	
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
	 * @return true if sound playing has finished.
	 * @return false otherwise.
	 */
	virtual bool FillSmpBuf(ting::Buffer<ting::s32>& buf, unsigned freq, unsigned chans) = 0;
};

}//~namespace



#include "Lib.hpp"



namespace aumiks{


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

}//~namespace
