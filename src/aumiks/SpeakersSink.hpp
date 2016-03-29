/**
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once

#include <audout/Player.hpp>

#include "Sink.hpp"

namespace aumiks{

//TODO: make singleton
template <audout::AudioFormat::EFrame frame_type> class SpeakersSink :
		public aumiks::ChanneledSink<frame_type>,
		private audout::Listener
{
	std::uint32_t freq;
	
	std::vector<Frame<frame_type>> smpBuf;
	
	audout::Player player;

	//this function is not thread-safe, but it is supposed to be called from special audio thread
	void fillPlayBuf(utki::Buf<std::int16_t> playBuf)noexcept override{
		ASSERT(this->smpBuf.size() == playBuf.size())

		if(this->input_var.fillSampleBuffer(utki::wrapBuf(this->smpBuf))){
			this->input_var.disconnect();
		}
		
		auto src = this->smpBuf.cbegin();
		auto dst = playBuf.begin();
		for(; src != this->smpBuf.cend(); ++src){
			for(unsigned i = 0; i != audout::AudioFormat::numChannels(frame_type); ++i, ++dst){
				ASSERT(playBuf.overlaps(dst))
				*dst = std::int16_t(utki::clampedRange(src->channel[i], -0x7fff, 0x7fff));
			}
		}
		ASSERT(dst == playBuf.end())
	}
	
public:
	SpeakersSink(audout::AudioFormat::ESamplingRate samplingRate, std::uint16_t bufferSizeMillis = 100) :
			freq(audout::AudioFormat(frame_type, samplingRate).frequency()),
			smpBuf((freq * bufferSizeMillis / 1000) * this->numChannels()),
			player(audout::AudioFormat(frame_type, samplingRate), smpBuf.size() / this->numChannels(), this)
	{}

	SpeakersSink(const SpeakersSink&) = delete;
	SpeakersSink& operator=(const SpeakersSink&) = delete;

	decltype(freq) frequency()const noexcept{
		return this->freq;
	}
	
	void start()override{
		this->player.setPaused(false);
	}

	void stop()override{
		this->player.setPaused(true);
	}
};



typedef SpeakersSink<audout::AudioFormat::EFrame::MONO> MonoSink;
typedef SpeakersSink<audout::AudioFormat::EFrame::STEREO> StereoSink;



}
