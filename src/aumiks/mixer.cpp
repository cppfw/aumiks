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

#include "mixer.hpp"

using namespace aumiks;

void mixer::connect(std::shared_ptr<aumiks::source> source){
	std::lock_guard<decltype(this->spin_lock)> guard(this->spin_lock);
	this->inputs_to_add.emplace_back();
	this->inputs_to_add.back().connect(std::move(source));
}

bool mixer::fill_sample_buffer(utki::span<frame> buf)noexcept{
	{
		std::lock_guard<decltype(this->spin_lock)> guard(this->spin_lock);
		this->inputs.splice(this->inputs.end(), this->inputs_to_add);
	}

	this->tmp_buf.resize(buf.size());

	for(auto& f : buf){
		for(auto& c : f.channel){
			c = 0;
		}
	}

	for(auto i = this->inputs.begin(); i != this->inputs.end();){
		if(i->fill_sample_buffer(utki::make_span(this->tmp_buf))){
			i = this->inputs.erase(i);
		}else{
			++i;
		}

		auto src = this->tmp_buf.cbegin();
		for(
				auto dst = buf.begin(),
						end = buf.end();
				dst != end;
				++dst,
						++src
			)
		{
			ASSERT(src != this->tmp_buf.cend())
			dst->add(*src);
		}
	}

	if(this->is_finite()){
		return this->inputs.size() == 0;
	}else{
		return false;
	}
}
