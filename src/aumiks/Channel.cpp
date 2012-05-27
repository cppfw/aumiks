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



#include "Channel.hpp"
#include "Lib.hpp"



using namespace aumiks;



bool Channel::FillSmpBufAndApplyEffects(ting::Buffer<ting::s32>& buf, unsigned freq, unsigned chans){
	ASSERT(buf.Size() % chans == 0)
	
	if(this->stopFlag){
		return true;
	}
	
	if(!this->soundStopped){
		this->soundStopped = this->FillSmpBuf(buf, freq, chans);
//		TRACE(<< "soundStopped = " << this->soundStopped << std::endl)
	}else{
		TRACE(<< "sound is stopped" << std::endl)
		//clear smp buffer
		memset(buf.Begin(), 0, buf.SizeInBytes());
	}

	//TODO:
	//Call channel effects.
	//If there are no any effects, it should return ch->soundStopped, otherwise, depending on the effects.
//	bool ret = this->ApplyEffectsToSmpBuf(ch);

	//TODO:
//	if(this->isMuted){
//		return ret;
//	}

	return this->soundStopped;//TODO: what to return?
}



void Channel::AddEffect_ts(const ting::Ref<aumiks::Effect>& effect){
	class AddEffectAction : public aumiks::Lib::Action{
		ting::Ref<aumiks::Channel> channel;
		ting::Ref<aumiks::Effect> effect;
		
		//override
		virtual void Perform(){
			//add the effect to sample buffer fillers chain
			if(this->channel->effects.size() == 0){
				this->effect->next = this->channel.operator->();
			}else{
				this->effect->next = this->channel->effects.back().operator->();
			}
			
			this->channel->effects.push_back(this->effect);
		}
		
	public:
		AddEffectAction(
				const ting::Ref<aumiks::Channel>& channel,
				const ting::Ref<aumiks::Effect>& effect
			) :
				channel(channel),
				effect(effect)
		{}
	};
	
	
	ting::Ptr<aumiks::Lib::Action> action(new AddEffectAction(
			ting::Ref<Channel>(this),
			effect
		));
	
	aumiks::Lib& lib = aumiks::Lib::Inst();
	
	ting::atomic::SpinLock::Guard mutexGuard(lib.actionsSpinLock);
	
	lib.addList->push_back(action);
}



void Channel::RemoveEffect_ts(const ting::Ref<aumiks::Effect>& effect){
	class RemoveEffectAction : public aumiks::Lib::Action{
		ting::Ref<aumiks::Channel> channel;
		ting::Ref<aumiks::Effect> effect;
		
		//override
		virtual void Perform(){
			for(aumiks::Effect::T_EffectsIter i = this->channel->effects.begin(); i != this->channel->effects.end();){
				if((*i) == effect){
					this->channel->effects.erase(i);
					//Now iterator points to next element in the list.
					
					//Remove effect from chain of sample buffer fillers
					if(i != this->channel->effects.end()){
						(*i)->next = this->effect->next;
					}
					return;
				}else{
					++i;
				}
			}
		}
		
	public:
		RemoveEffectAction(
				const ting::Ref<aumiks::Channel>& channel,
				const ting::Ref<aumiks::Effect>& effect
			) :
				channel(channel),
				effect(effect)
		{}
	};
	
	
	ting::Ptr<aumiks::Lib::Action> action(new RemoveEffectAction(
			ting::Ref<Channel>(this),
			effect
		));
	
	aumiks::Lib& lib = aumiks::Lib::Inst();
	
	ting::atomic::SpinLock::Guard mutexGuard(lib.actionsSpinLock);
	
	lib.addList->push_back(action);
}



void Channel::RemoveAllEffects_ts(){
	class RemoveAllEffectsAction : public aumiks::Lib::Action{
		ting::Ref<aumiks::Channel> channel;
		
		//override
		virtual void Perform(){
			this->channel->effects.clear();
		}
		
	public:
		RemoveAllEffectsAction(
				const ting::Ref<aumiks::Channel>& channel
			) :
				channel(channel)
		{}
	};
	
	ting::Ptr<aumiks::Lib::Action> action(new RemoveAllEffectsAction(
			ting::Ref<Channel>(this)
		));
	
	aumiks::Lib& lib = aumiks::Lib::Inst();
	
	ting::atomic::SpinLock::Guard mutexGuard(lib.actionsSpinLock);
	
	lib.addList->push_back(action);
}



void Channel::Play(){
	aumiks::Lib::Inst().PlayChannel_ts(ting::Ref<aumiks::Channel>(this));
}


