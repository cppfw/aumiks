#pragma once

#include "Frame.hpp"
#include "Source.hpp"
#include "Input.hpp"

#include <vector>

namespace aumiks{

class AReframer : virtual public ASource{
protected:
	AReframer(){}
public:
	virtual Input& input() = 0;
};

template <audout::Frame_e from_type, audout::Frame_e to_type> class Reframer :
		public Source<to_type>,
		public AReframer
	{
	
	void fillSampleBuffer_i(utki::Buf<Frame<to_type>> buf)noexcept;
	
	std::vector<Frame<from_type>> tmpBuf;
	
	ChanneledInput<from_type> input_v;
public:
	Input& input()override{
		return this->input_v;
	}
	
	bool fillSampleBuffer(utki::Buf<Frame<to_type>> buf)noexcept override{
		if(this->tmpBuf.size() != buf.size()){
			this->tmpBuf.resize(buf.size());
		}
		
		bool ret = this->input_v.fillSampleBuffer(utki::wrapBuf(this->tmpBuf));
		
		this->fillSampleBuffer_i(buf);
		
		return ret;
	}
	
};
	
}
