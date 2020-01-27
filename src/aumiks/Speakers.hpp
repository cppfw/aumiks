#pragma once

#include <audout/player.hpp>

#include "Sink.hpp"

namespace aumiks{

//TODO: make singleton
class Speakers :
		public aumiks::Sink,
		private audout::listener
{
	std::vector<Frame> smpBuf;

	// this function is not thread-safe, but it is supposed to be called from special audio thread
	void fill(utki::span<std::int16_t> playBuf)noexcept override;
	
public:
	const std::uint32_t samplingRate;
	
private:
	audout::player player;
	
public:
	Speakers(audout::sampling_rate samplingRate, uint16_t bufferSizeMillis = 100);

	Speakers(const Speakers&) = delete;
	Speakers& operator=(const Speakers&) = delete;
	
	void start()override;

	void stop()override;
};

}
