#include "SpeakersSink.hpp"

using namespace aumiks;

void SpeakersSink::start() {
	this->player.setPaused(false);
}

void SpeakersSink::stop() {
	this->player.setPaused(true);
}

SpeakersSink::SpeakersSink(audout::SamplingRate_e samplingRate, std::uint16_t bufferSizeMillis) :
		samplingRate(audout::AudioFormat(audout::Frame_e::STEREO, samplingRate).frequency()),
		player(audout::AudioFormat(audout::Frame_e::STEREO, samplingRate), (this->samplingRate * bufferSizeMillis / 1000), this)
{}


void SpeakersSink::fillPlayBuf(utki::Buf<std::int16_t> playBuf) noexcept{
	ASSERT_INFO(
			playBuf.size() % audout::AudioFormat::numChannels(audout::Frame_e::STEREO) == 0,
			"playBuf.size = " << playBuf.size()
			<< "numChannels = " << audout::AudioFormat::numChannels(audout::Frame_e::STEREO)
		)
	this->smpBuf.resize(playBuf.size() / audout::AudioFormat::numChannels(audout::Frame_e::STEREO));

	if (this->input.fillSampleBuffer(utki::wrapBuf(this->smpBuf))) {
		this->input.disconnect();
	}

	auto src = this->smpBuf.cbegin();
	auto dst = playBuf.begin();
	for (; src != this->smpBuf.cend(); ++src) {
		for (unsigned i = 0; i != src->channel.size(); ++i, ++dst) {
			ASSERT(playBuf.overlaps(dst))
					*dst = std::int16_t(utki::clampedRange(std::int32_t(src->channel[i]), -0x7fff, 0x7fff));
		}
	}
	ASSERT(dst == playBuf.end())
}
