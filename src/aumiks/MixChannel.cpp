#include "MixChannel.hpp"
#include "Lib.hpp"



using namespace aumiks;



void MixChannel::MixSmpBufTo(utki::Buf<std::int32_t>& buf){
	ASSERT(this->smpBuf.Size() == buf.Size())

	std::int32_t* src = this->smpBuf.Begin();
	std::int32_t* dst = buf.Begin();

	for(; dst != buf.End(); ++src, ++dst){
		*dst += *src;
	}
}



bool MixChannel::FillSmpBuf(utki::Buf<std::int32_t>& buf){
	ASSERT(buf.Size() % aumiks::Lib::Inst().OutputFormat().frame.NumChannels() == 0)
	
	//check if this mix channel holds sample buffer of a correct size
	//TODO: assign buffer in the audio thread when channel starts to play
	if(this->smpBuf.Size() != buf.Size()){
		this->smpBuf.Init(buf.Size());
	}
	
	T_ChannelIter i = this->channels.begin();
	if(i != this->channels.end()){//if there is at least one child channel
		//the very first channel is not mixed, but simply written to the output buffer
		if((*i)->FillSmpBufAndApplyEffects(buf)){
			(*i)->stoppedFlag = true;
			i = this->channels.erase(i);
		}else{
			++i;
		}

		for(; i != this->channels.end();){
			if((*i)->FillSmpBufAndApplyEffects(this->smpBuf)){
				(*i)->stoppedFlag = true;
				i = this->channels.erase(i);
			}else{
				++i;
			}
			this->MixSmpBufTo(buf);
		}

		return !this->isPersistent && (this->channels.size() == 0);
	}else{//no any child channels to play initially
		if(!this->isPersistent){
			return true;
		}

		//zero out the sample buffer
		memset(buf.Begin(), 0, buf.SizeInBytes());
		return false;
	}
}



void MixChannel::PlayChannel_ts(const std::shared_ptr<aumiks::Channel>& channel){
	class PlayChannelAction : public aumiks::Lib::Action{
		std::shared_ptr<aumiks::MixChannel> mixChannel;
		std::shared_ptr<aumiks::Channel> channelToPlay;
		
		//override
		virtual void Perform(){
			this->mixChannel->channels.push_back(this->channelToPlay);
		}
		
	public:
		PlayChannelAction(
				const std::shared_ptr<aumiks::MixChannel>& mixChannel,
				const std::shared_ptr<aumiks::Channel>& channelToPlay
			) :
				mixChannel(mixChannel),
				channelToPlay(channelToPlay)
		{}
	};//~class

	aumiks::Lib::Inst().PushAction_ts(std::unique_ptr<aumiks::Lib::Action>(
			new PlayChannelAction(
					std::shared_ptr<MixChannel>(this),
					channel
				)
		));
}
