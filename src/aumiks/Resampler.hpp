#pragma once

#include "Source.hpp"
#include "Input.hpp"

namespace aumiks{

template <audout::AudioFormat::EFrame frame_type> class Resampler : ChanneledSource<frame_type>{
public:
	Resampler(const Resampler&) = delete;
	Resampler& operator=(const Resampler&) = delete;
	
	
private:

};

}
