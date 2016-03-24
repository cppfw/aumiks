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
	std::vector<std::int32_t> smpBuf;
	
	audout::Player player;
	
	std::uint32_t freq;

	//this function is not thread-safe, but it is supposed to be called from special audio thread
	void fillPlayBuf(utki::Buf<std::int16_t> playBuf)noexcept override{
		ASSERT(this->smpBuf.size() == playBuf.size())

		if(this->input_var.FillSampleBuffer(utki::wrapBuf(this->smpBuf))){
			this->input_var.Disconnect();
		}
		
		auto src = this->smpBuf.cbegin();
		auto dst = playBuf.begin();
		for(; src != this->smpBuf.cend(); ++src, ++dst){
			std::int32_t tmp = *src;
			utki::clampTop(tmp, 0x7fff);
			utki::clampBottom(tmp, -0x7fff);

			ASSERT(playBuf.overlaps(dst))
			
			*dst = std::int16_t(tmp);
		}
		ASSERT(dst == playBuf.end())
	}
	
public:
	SpeakersSink(audout::AudioFormat::ESamplingRate samplingRate, std::uint16_t bufferSizeMillis = 100) :
			freq(audout::AudioFormat(frame_type, samplingRate).frequency()),
			smpBuf((freq * bufferSizeMillis / 1000) * this->NumChannels()),
			player(audout::AudioFormat(frame_type, samplingRate), smpBuf.size() / this->NumChannels(), this)
	{}

	SpeakersSink(const SpeakersSink&) = delete;
	SpeakersSink& operator=(const SpeakersSink&) = delete;

	decltype(freq) frequency()const noexcept{
		return this->freq;
	}
	
	void Start()override{
		this->player.setPaused(false);
	}

	void Stop()override{
		this->player.setPaused(true);
	}
};



typedef SpeakersSink<audout::AudioFormat::EFrame::MONO> MonoSink;
typedef SpeakersSink<audout::AudioFormat::EFrame::STEREO> StereoSink;



}
