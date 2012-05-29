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
			
			this->channel->lastFillerInChain = this->channel->effects.back().operator->();
			
			//if the channel is currently playing then initialize the newly added effect
			if(this->channel->isPlaying){
				this->effect->Init_ts();
			}
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
	
	
	aumiks::Lib::Inst().PushAction_ts(ting::Ptr<aumiks::Lib::Action>(
			new AddEffectAction(
					ting::Ref<Channel>(this),
					effect
				)
		));
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
					}else{
						this->channel->lastFillerInChain = this->channel->effects.back().operator->();
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
	
	
	aumiks::Lib::Inst().PushAction_ts(ting::Ptr<aumiks::Lib::Action>(
			new RemoveEffectAction(
					ting::Ref<Channel>(this),
					effect
				)
		));
}



void Channel::RemoveAllEffects_ts(){
	class RemoveAllEffectsAction : public aumiks::Lib::Action{
		ting::Ref<aumiks::Channel> channel;
		
		//override
		virtual void Perform(){
			this->channel->effects.clear();
			this->channel->lastFillerInChain = this->channel.operator->();
		}
		
	public:
		RemoveAllEffectsAction(
				const ting::Ref<aumiks::Channel>& channel
			) :
				channel(channel)
		{}
	};
	
	aumiks::Lib::Inst().PushAction_ts(ting::Ptr<aumiks::Lib::Action>(
			new RemoveAllEffectsAction(
					ting::Ref<Channel>(this)
				)
		));
}



void Channel::Play(){
	aumiks::Lib::Inst().PlayChannel_ts(ting::Ref<aumiks::Channel>(this));
}


