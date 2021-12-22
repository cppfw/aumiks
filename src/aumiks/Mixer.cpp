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

#include "Mixer.hpp"

using namespace aumiks;

void Mixer::connect(std::shared_ptr<Source> source) {
	std::lock_guard<decltype(this->spinLock) > guard(this->spinLock);
	this->inputsToAdd.emplace_back();
	this->inputsToAdd.back().connect(std::move(source));
}

bool Mixer::fillSampleBuffer(utki::span<frame> buf) noexcept{
	{
		std::lock_guard<decltype(this->spinLock)> guard(this->spinLock);
		this->inputs.splice(this->inputs.end(), this->inputsToAdd);
	}

	this->tmpBuf.resize(buf.size());

	for(auto& f : buf){
		for(auto& c : f.channel){
			c = 0;
		}
	}

	for(auto i = this->inputs.begin(); i != this->inputs.end();){
		if(i->fill_sample_buffer(utki::make_span(this->tmpBuf))){
			i = this->inputs.erase(i);
		}else{
			++i;
		}

		auto src = this->tmpBuf.cbegin();
		for(
				auto dst = buf.begin(),
						end = buf.end();
				dst != end;
				++dst,
						++src
			)
		{
			ASSERT(src != this->tmpBuf.cend())
			dst->add(*src);
		}
	}

	if(this->isFinite()){
		return this->inputs.size() == 0;
	}else{
		return false;
	}
}
