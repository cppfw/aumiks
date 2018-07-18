#pragma once

#include "Source.hpp"
#include "Input.hpp"
#include "SingleInputSource.hpp"

namespace aumiks{

class Resampler : public SingleInputSource{
	static const std::uint16_t DScale = 128;
	
	std::int32_t scale = 0;
	
	volatile std::uint16_t step = DScale;
	
	
public:
	Resampler(const Resampler&) = delete;
	Resampler& operator=(const Resampler&) = delete;
	
	Resampler(){}
	
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
	std::vector<Frame> tmpBuf;
	Frame lastFrameForUpsampling;
	
	bool fillSampleBuffer(utki::Buf<Frame> buf)noexcept override;
};

}
