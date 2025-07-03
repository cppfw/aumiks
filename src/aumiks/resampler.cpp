/*
The MIT License (MIT)

Copyright (c) 2011-2024 Ivan Gagis <igagis@gmail.com>

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

#include "resampler.hpp"

using namespace aumiks;

bool resampler::fill_sample_buffer(utki::span<frame> buf) noexcept
{
	ASSERT(this->step > 0) // if step is 0 then there will be infinite loop

	// variable step can be changed from another thread, so copy it here
	typename std::remove_volatile<decltype(this->step)>::type s = this->step;

	auto dst = buf.begin();

	if (s > no_resample_step) { // if up-sampling
		size_t filled_from_prev_call = 0;
		// something has left from previous call
		for (; this->scale > 0 && dst != buf.end(); scale -= no_resample_step, ++dst, ++filled_from_prev_call) {
			*dst = this->last_frame_for_upsampling;
		}
		if (dst == buf.end()) {
			return false;
		}
		this->tmp_buf.resize((buf.size() - filled_from_prev_call) * no_resample_step / s + 1);
	} else {
		this->tmp_buf.resize((buf.size()) * no_resample_step / s);
	}

	bool ret = this->input.fill_sample_buffer(utki::make_span(this->tmp_buf));

	auto src = this->tmp_buf.cbegin();
	for (; dst != buf.end(); ++src) {
		this->scale += s;
		for (; this->scale > 0 && dst != buf.end(); this->scale -= no_resample_step, ++dst) {
			ASSERT(dst != buf.end(), [&](auto& o) {
				o << "s = " << s << " buf.size() = " << buf.size() << " this->tmp_buf.size() = " << this->tmp_buf.size()
				  << " this->scale = " << this->scale << " dst-end = " << (dst - buf.end());
			})
			ASSERT(src != this->tmp_buf.cend(), [&](auto& o) {
				o << "s = " << s << " buf.size() = " << buf.size() << " this->tmp_buf.size() = " << this->tmp_buf.size()
				  << " this->scale = " << this->scale << " dst-end = " << (dst - buf.end());
			})
			*dst = *src;
		}
	}
	ASSERT(dst == buf.end())

	if (src != this->tmp_buf.cend()) {
		// one more sample left in source buffer
		ASSERT(src + 1 == this->tmp_buf.cend(), [&](auto& o) {
			o << "s = " << s << " buf.size() = " << buf.size() << " this->tmp_buf.size() = " << this->tmp_buf.size()
			  << " this->scale = " << this->scale << " src-end = " << (src - this->tmp_buf.cend());
		})
		this->scale += s;
	}

	if (this->scale > 0) {
		ASSERT(s > no_resample_step) // was upsampling
		this->last_frame_for_upsampling = this->tmp_buf.back();
	}

	return ret;
}
