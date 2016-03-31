#pragma once

#include "Source.hpp"
#include "Input.hpp"

namespace aumiks{


class Resampler : virtual public Source{
protected:
	Resampler(){}
public:
	virtual Input& input() = 0;
};


template <audout::Frame_e frame_type> class ChanneledResampler :
		public ChanneledSource<frame_type>,
		public Resampler
{
	static const std::uint16_t DScale = 128;
	
	std::int32_t scale = 0;
	
	volatile std::uint16_t step = DScale;
	
	ChanneledInput<frame_type> input_v;
public:
	ChanneledResampler(const ChanneledResampler&) = delete;
	ChanneledResampler& operator=(const ChanneledResampler&) = delete;
	
	ChanneledResampler(){}
	
	Input& input() override{
		return this->input_v;
	}
	
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
	std::vector<Frame<frame_type>> tmpBuf;
	Frame<frame_type> lastFrameForUpsampling;
	
	bool fillSampleBuffer(utki::Buf<Frame<frame_type>> buf)noexcept override{
		ASSERT(this->step > 0) //if step is 0 then there will be infinite loop
		
		//variable step can be changed from another thread, so copy it here
		typename std::remove_volatile<decltype(this->step)>::type s = this->step;
		
		auto dst = buf.begin();
		
		{
			size_t filledFromPrevCall = 0;

			if(this->scale > 0){
				if(s > DScale){//if up-sampling
					//something has left from previous call
					for(; this->scale > 0 && dst != buf.end(); scale -= DScale, ++dst, ++filledFromPrevCall){
						*dst = this->lastFrameForUpsampling;
					}
					if(dst == buf.end()){
						return false;
					}
				}else{
					this->scale = 0;
				}
			}
			
			this->tmpBuf.resize((buf.size() - filledFromPrevCall) * DScale / s);
		}
		
		bool ret = this->input_v.fillSampleBuffer(utki::wrapBuf(this->tmpBuf));
		
		if(s > DScale){//if up-sampling
			for(auto src = this->tmpBuf.cbegin(); src != this->tmpBuf.cend(); ++src){
				ASSERT(src != this->tmpBuf.cend())
				ASSERT(dst != buf.end())
				
				ASSERT(this->scale <= 0)
				
				for(this->scale += s; this->scale > 0 && dst != buf.end(); this->scale -= DScale, ++dst){
					*dst = *src;
				}
				ASSERT((dst != buf.end()) || (src + 1 == this->tmpBuf.cend()))
			}
			if(this->scale > 0){
				this->lastFrameForUpsampling = this->tmpBuf.back();
			}
		}else{// if down-sampling
			ASSERT(s <= DScale)

			for(auto src = this->tmpBuf.cbegin(); dst != buf.end(); ++dst){
				ASSERT(src != this->tmpBuf.cend())
				ASSERT(this->scale <= 0)
				
				*dst = *src;

				for(this->scale += DScale; this->scale > 0; ++src, this->scale -= s){
					ASSERT(src != this->tmpBuf.cend())
				}
			}
		}
		//TODO: dst + 1 shouldn be here
		ASSERT(dst == buf.end() || dst + 1 == buf.end())
		
		return ret;
	}
};

}
