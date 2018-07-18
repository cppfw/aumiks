#pragma once

#include <audout/Player.hpp>

#include "Sink.hpp"

namespace aumiks{

//TODO: make singleton
class SpeakersSink :
		public aumiks::Sink,
		private audout::Listener
{
	std::uint32_t samplingRate_v;
	
	std::vector<Frame> smpBuf;
	
	audout::Player player;

	//this function is not thread-safe, but it is supposed to be called from special audio thread
	void fillPlayBuf(utki::Buf<std::int16_t> playBuf)noexcept override;
	
public:
	SpeakersSink(audout::SamplingRate_e samplingRate, std::uint16_t bufferSizeMillis = 100);

	SpeakersSink(const SpeakersSink&) = delete;
	SpeakersSink& operator=(const SpeakersSink&) = delete;

	decltype(samplingRate_v) samplingRate()const noexcept{
		return this->samplingRate_v;
	}
	
	void start()override{
		this->player.setPaused(false);
	}

	void stop()override{
		this->player.setPaused(true);
	}
};

}
