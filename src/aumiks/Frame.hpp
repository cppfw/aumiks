#pragma once

#include <audout/AudioFormat.hpp>

#include <array>

#include <utki/debug.hpp>

#include "config.hpp"

namespace aumiks{

struct Frame{
	std::array<real, audout::AudioFormat::numChannels(audout::Frame_e::STEREO)> channel;
	
	void add(const Frame& f){
		ASSERT(this->channel.size() == f.channel.size())
		for(unsigned i = 0; i != this->channel.size(); ++i){
			this->channel[i] += f.channel[i];
		}
	}
};

}
