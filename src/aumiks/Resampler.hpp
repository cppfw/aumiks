#pragma once

#include "Source.hpp"
#include "Input.hpp"

namespace aumiks{

template <audout::AudioFormat::EFrame frame_type> class Resampler : public ChanneledSource<frame_type>{
	static const std::uint16_t DScale = 32;
	
	std::int32_t scale;
	
	std::uint16_t step = DScale;
	
public:
	Resampler(const Resampler&) = delete;
	Resampler& operator=(const Resampler&) = delete;
	
	Resampler(){
		this->reset();
	}
	
	ChanneledInput<frame_type> input;
	
	void setScale(float scale)noexcept{
		this->step = decltype(step)(scale * float(DScale));
	}
	
	void setScale(std::uint32_t fromSamplingRate, std::uint32_t toSamplingRate){
		if(fromSamplingRate == 0){
			return;
		}
		this->step = toSamplingRate * DScale / fromSamplingRate;
	}
	
private:
	static const std::size_t DTmpBufSize = 50 * 48000 / 1000; //for 50ms of sound data at 48kHz
	std::array<Frame<frame_type>, DTmpBufSize> tmpBuf; //TODO: use vector
	typename decltype(tmpBuf)::iterator curTmpPos;
	
	void reset(){
		this->scale = 0;
		this->curTmpPos = this->tmpBuf.end();
	}
	
	bool fillSampleBuffer(utki::Buf<Frame<frame_type>> buf)noexcept override{
		ASSERT(this->step > 0) //if step is 0 then there will be infinite loop
		
		bool ret = false;
		
		auto dst = buf.begin();
		
		if(this->step > DScale){//if up-sampling
			for(; dst != buf.end() && !ret; ++this->curTmpPos){
				if(this->curTmpPos == this->tmpBuf.end()){
					if(ret){
						break;
					}
					ret = this->input.fillSampleBuffer(utki::wrapBuf(this->tmpBuf));
					this->curTmpPos = this->tmpBuf.begin();
				}
				if(this->scale <= 0){
					this->scale += this->step;
				}
				for(; this->scale > 0;){
					*dst = *this->curTmpPos;
					
					this->scale -= DScale;
					++dst;
					if(dst == buf.end()){
						goto endloop; //to avoid incrementing of this->curTmpPos
					}
				}
			}
		}else{// if down-sampling
			ASSERT(this->step <= DScale)
			
			for(; dst != buf.end() && !ret;){
				if(this->curTmpPos == this->tmpBuf.end()){
					if(ret){
						break;
					}
					ret = this->input.fillSampleBuffer(utki::wrapBuf(this->tmpBuf));
					this->curTmpPos = this->tmpBuf.begin();
				}
				if(this->scale <= 0){
					this->scale += DScale;
					*dst = *this->curTmpPos;
					++dst;
				}
				for(; this->scale > 0 && this->curTmpPos != this->tmpBuf.end(); ++this->curTmpPos, this->scale -= this->step){
				}
			}
		}
endloop:
		for(; dst != buf.end(); ++dst){
			for(auto& c : dst->channel){
				c = 0;
			}
		}
		if(ret){
			this->reset();
		}
		
		return ret;
	}
};

}
