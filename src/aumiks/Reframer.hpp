#pragma once

#include "Frame.hpp"
#include "Source.hpp"
#include "Input.hpp"

#include <vector>

namespace aumiks{
	
template <audout::AudioFormat::EFrame from_type, audout::AudioFormat::EFrame to_type> class Reframer : public ChanneledSource<to_type>{
	
	
public:
	ChanneledInput<from_type> input;
	
	bool fillSampleBuffer(utki::Buf<Frame<to_type>> buf)noexcept override{
		return true;
	}
};
	
}
