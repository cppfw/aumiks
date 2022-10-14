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

#include "speakers.hpp"

using namespace aumiks;

void speakers::start() {
	this->player.set_paused(false);
}

void speakers::stop() {
	this->player.set_paused(true);
}

speakers::speakers(audout::rate sampling_rate, uint16_t buffer_size_millis) :
		sampling_rate(audout::format(audout::frame::stereo, sampling_rate).frequency()),
		player(audout::format(audout::frame::stereo, sampling_rate), (this->sampling_rate * buffer_size_millis / 1000), this)
{}

void speakers::fill(utki::span<std::int16_t> play_buf)noexcept{
	ASSERT(
		play_buf.size() % audout::num_channels(audout::frame::stereo) == 0,
		[&](auto&o){
			o << "play_buf.size() = " << play_buf.size()
			<< "num_channels = " << audout::num_channels(audout::frame::stereo);
		}
	)
	this->smp_buf.resize(play_buf.size() / audout::num_channels(audout::frame::stereo));

	if(this->input.fill_sample_buffer(utki::make_span(this->smp_buf))){
		this->input.disconnect();
	}

	auto src = this->smp_buf.cbegin();
	auto dst = play_buf.begin();
	for(; src != this->smp_buf.cend(); ++src){
		for(unsigned i = 0; i != src->channel.size(); ++i, ++dst){
			ASSERT(play_buf.overlaps(dst))
			using std::min;
			using std::max;
			*dst = int16_t(max(-0x7fff, min(int32_t(src->channel[i]), 0x7fff)));
		}
	}
	ASSERT(dst == play_buf.end())
}
