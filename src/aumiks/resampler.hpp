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

#pragma once

#include "source.hpp"
#include "input.hpp"
#include "single_input_source.hpp"

namespace aumiks{

class resampler : public single_input_source{
	static const uint16_t no_resample_step = 128;
	
	int32_t scale = 0;
	
	volatile uint16_t step = no_resample_step;
	
public:
	resampler(const resampler&) = delete;
	resampler& operator=(const resampler&) = delete;
	
	resampler(){}
	
	void set_scale(float scale)noexcept{
		this->step = decltype(step)(scale * float(no_resample_step));
	}
	
	void set_scale(uint32_t from_sampling_rate, uint32_t to_sampling_rate){
		if(from_sampling_rate == 0){
			return;
		}
		this->step = to_sampling_rate * no_resample_step / from_sampling_rate;
	}
	
private:
	std::vector<frame> tmp_buf;
	frame last_frame_for_upsampling;
	
	bool fill_sample_buffer(utki::span<frame> buf)noexcept override;
};

}
