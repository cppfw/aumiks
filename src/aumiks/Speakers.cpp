#include "Speakers.hpp"

using namespace aumiks;

void Speakers::start() {
	this->player.set_paused(false);
}

void Speakers::stop() {
	this->player.set_paused(true);
}

Speakers::Speakers(audout::rate samplingRate, uint16_t bufferSizeMillis) :
		samplingRate(audout::format(audout::frame::stereo, samplingRate).frequency()),
		player(audout::format(audout::frame::stereo, samplingRate), (this->samplingRate * bufferSizeMillis / 1000), this)
{}


void Speakers::fill(utki::span<std::int16_t> playBuf) noexcept{
	ASSERT_INFO(
			playBuf.size() % audout::AudioFormat::numChannels(audout::frame::stereo) == 0,
			"playBuf.size = " << playBuf.size()
			<< "numChannels = " << audout::AudioFormat::numChannels(audout::frame::stereo)
		)
	this->smpBuf.resize(playBuf.size() / audout::num_channels(audout::frame::stereo));

	if (this->input.fillSampleBuffer(utki::make_span(this->smpBuf))) {
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
