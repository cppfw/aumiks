#pragma once

#include <audout/Player.hpp>

#include "Sink.hpp"

namespace aumiks{

//TODO: make singleton
class SpeakersSink :
		public aumiks::Sink,
		private audout::Listener
{
	std::vector<Frame> smpBuf;

	//this function is not thread-safe, but it is supposed to be called from special audio thread
	void fillPlayBuf(utki::Buf<std::int16_t> playBuf)noexcept override;
	
public:
	const std::uint32_t samplingRate;
	
private:
	audout::Player player;
	
public:
	SpeakersSink(audout::SamplingRate_e samplingRate, std::uint16_t bufferSizeMillis = 100);

	SpeakersSink(const SpeakersSink&) = delete;
	SpeakersSink& operator=(const SpeakersSink&) = delete;
	
	void start()override;

	void stop()override;
};

}
