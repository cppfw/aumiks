/*
The MIT License (MIT)

Copyright (c) 2011-2021 Ivan Gagis <igagis@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

/* ================ LICENSE END ================ */

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
			playBuf.size() % audout::num_channels(audout::frame::stereo) == 0,
			"playBuf.size = " << playBuf.size()
			<< "numChannels = " << audout::num_channels(audout::frame::stereo)
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
			using std::min;
			using std::max;
			*dst = int16_t(max(-0x7fff, min(int32_t(src->channel[i]), 0x7fff)));
		}
	}
	ASSERT(dst == playBuf.end())
}
