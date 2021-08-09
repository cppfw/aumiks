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

#include "Source.hpp"
#include "Input.hpp"
#include "SingleInputSource.hpp"

namespace aumiks{

class Resampler : public SingleInputSource{
	static const std::uint16_t DScale = 128;
	
	std::int32_t scale = 0;
	
	volatile std::uint16_t step = DScale;
	
	
public:
	Resampler(const Resampler&) = delete;
	Resampler& operator=(const Resampler&) = delete;
	
	Resampler(){}
	
	void setScale(float scale)noexcept{
		this->step = decltype(step)(scale * float(DScale));
	}
	
	void setScale(std::uint32_t fromSamplingRate, std::uint32_t toSamplingRate){
		if(fromSamplingRate == 0){
			return;
		}
		this->step = toSamplingRate * DScale / fromSamplingRate;
	}
	
private:
	std::vector<Frame> tmpBuf;
	Frame lastFrameForUpsampling;
	
	bool fillSampleBuffer(utki::span<Frame> buf)noexcept override;
};

}
