#pragma once

#include <audout/AudioFormat.hpp>

#include <array>

namespace aumiks{

template <audout::AudioFormat::EFrame frame_type> struct Frame{
	std::array<std::int32_t, audout::AudioFormat::numChannels(frame_type)> channel;
};

}
