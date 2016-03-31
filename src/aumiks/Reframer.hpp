#pragma once

#include "Frame.hpp"
#include "Source.hpp"
#include "Input.hpp"

#include <vector>

namespace aumiks{
	
template <audout::AudioFormat::EFrame from_type, audout::AudioFormat::EFrame to_type> class Reframer : public ChanneledSource<to_type>{
	
	void fillSampleBuffer_i(utki::Buf<Frame<to_type>> buf)noexcept;
	
	std::vector<Frame<from_type>> tmpBuf;
public:
	ChanneledInput<from_type> input;
	
	bool fillSampleBuffer(utki::Buf<Frame<to_type>> buf)noexcept override{
		if(this->tmpBuf.size() != buf.size()){
			this->tmpBuf.resize(buf.size());
		}
		
		bool ret = this->input.fillSampleBuffer(utki::wrapBuf(this->tmpBuf));
		
		this->fillSampleBuffer_i(buf);
		
		return ret;
	}
	
};
	
}
