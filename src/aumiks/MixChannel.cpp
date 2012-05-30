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



#include "MixChannel.hpp"
#include "Lib.hpp"



using namespace aumiks;



void MixChannel::MixSmpBufTo(ting::Buffer<ting::s32>& buf){
	ASSERT(this->smpBuf.Size() == buf.Size())

	ting::s32* src = this->smpBuf.Begin();
	ting::s32* dst = buf.Begin();

	for(; dst != buf.End(); ++src, ++dst){
		*dst += *src;
	}
}



bool MixChannel::FillSmpBuf(ting::Buffer<ting::s32>& buf, unsigned freq, unsigned chans){
	//check if this mix channel holds sample buffer of a correct size
	//TODO: assign buffer in the audio thread when channel starts to play
	if(this->smpBuf.Size() != buf.Size()){
		this->smpBuf.Init(buf.Size());
	}
	
	{
		T_ChannelIter i = this->channels.begin();
		if(i != this->channels.end()){
			//the very first channel is not mixed, but simply written to the output buffer
			if((*i)->FillSmpBufAndApplyEffects(buf, freq, chans)){
				(*i)->isPlaying = false;//clear playing flag
				(*i)->OnStop();//notify channel that it has stopped playing
				i = this->channels.erase(i);
			}else{
				++i;
			}
			
			for(; i != this->channels.end();){
				if((*i)->FillSmpBufAndApplyEffects(this->smpBuf, freq, chans)){
					(*i)->isPlaying = false;//clear playing flag
					(*i)->OnStop();//notify channel that it has stopped playing
					i = this->channels.erase(i);
				}else{
					++i;
				}
				this->MixSmpBufTo(buf);
			}
		}
	}

	return !this->isPersistent && (this->channels.size() == 0);
}



void MixChannel::PlayChannel_ts(const ting::Ref<aumiks::Channel>& channel){
	class PlayChannelAction : public aumiks::Lib::Action{
		ting::Ref<aumiks::MixChannel> mixChannel;
		ting::Ref<aumiks::Channel> channelToPlay;
		
		//override
		virtual void Perform(){			
			this->mixChannel->channels.push_back(this->channelToPlay);
			
			this->channelToPlay->InitEffects();
			
			//notify channel that it has started playing
			this->channelToPlay->OnStart();
		}
		
	public:
		PlayChannelAction(
				const ting::Ref<aumiks::MixChannel>& mixChannel,
				const ting::Ref<aumiks::Channel>& channelToPlay
			) :
				mixChannel(mixChannel),
				channelToPlay(channelToPlay)
		{}
	};//~class
	
	//mark channel as playing before sending the action because the action may be
	//handled before setting the flag if it is set after sending the action!
	channel->isPlaying = true;
	
	try{
		aumiks::Lib::Inst().PushAction_ts(ting::Ptr<aumiks::Lib::Action>(
				new PlayChannelAction(
						ting::Ref<MixChannel>(this),
						channel
					)
			));
	}catch(...){
		channel->isPlaying = false;
		throw;
	}
}
