#pragma once

#include <audout/Player.hpp>

#include "Sink.hpp"

namespace aumiks{

//TODO: make singleton
template <audout::Frame_e frame_type> class SpeakersSink :
		public aumiks::FramedSink<std::int32_t, frame_type>,
		private audout::Listener
{
	std::uint32_t samplingRate_v;
	
	std::vector<Frame<std::int32_t, frame_type>> smpBuf;
	
	audout::Player player;

	//this function is not thread-safe, but it is supposed to be called from special audio thread
	void fillPlayBuf(utki::Buf<std::int16_t> playBuf)noexcept override{
		ASSERT_INFO(
				playBuf.size() % audout::AudioFormat::numChannels(frame_type) == 0,
				"playBuf.size = " << playBuf.size()
						<< "numChannels = " << audout::AudioFormat::numChannels(frame_type)
			)
		this->smpBuf.resize(playBuf.size() / audout::AudioFormat::numChannels(frame_type));

		if(this->input_var.fillSampleBuffer(utki::wrapBuf(this->smpBuf))){
			this->input_var.disconnect();
		}
		
		auto src = this->smpBuf.cbegin();
		auto dst = playBuf.begin();
		for(; src != this->smpBuf.cend(); ++src){
			for(unsigned i = 0; i != src->channel.size(); ++i, ++dst){
				ASSERT(playBuf.overlaps(dst))
				*dst = std::int16_t(utki::clampedRange(src->channel[i], -0x7fff, 0x7fff));
			}
		}
		ASSERT(dst == playBuf.end())
	}
	
public:
	SpeakersSink(audout::SamplingRate_e samplingRate, std::uint16_t bufferSizeMillis = 100) :
			samplingRate_v(audout::AudioFormat(frame_type, samplingRate).frequency()),
			player(audout::AudioFormat(frame_type, samplingRate), (samplingRate_v * bufferSizeMillis / 1000), this)
	{}

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



typedef SpeakersSink<audout::Frame_e::MONO> MonoSink;
typedef SpeakersSink<audout::Frame_e::STEREO> StereoSink;



}
