#pragma once

#include <audout/format.hpp>

#include <array>

#include <utki/debug.hpp>

#include "config.hpp"

namespace aumiks{

struct Frame{
	std::array<real, audout::format::num_channels(audout::frame_type::stereo)> channel;
	
	void add(const Frame& f){
		ASSERT(this->channel.size() == f.channel.size())
		for(unsigned i = 0; i != this->channel.size(); ++i){
			this->channel[i] += f.channel[i];
		}
	}
};

}
