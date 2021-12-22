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

#include "Input.hpp"

using namespace aumiks;



void Input::connect(std::shared_ptr<aumiks::Source> source){
	ASSERT(source)

	if(this->isConnected()){
		throw std::logic_error("Input already connected");
	}

	if(source->isConnected()){
		throw std::logic_error("Source is already connected");
	}
	
	{
		std::lock_guard<decltype(this->mutex)> guard(this->mutex);
		source->isConnected_v = true;
		this->src = std::move(source);
	}
}

void Input::disconnect()noexcept {
	std::lock_guard<decltype(this->mutex) > guard(this->mutex);

	if (this->src) {
		this->src->isConnected_v = false;
		this->src.reset();
	}
}


bool Input::fillSampleBuffer(utki::span<frame> buf) noexcept{
	{
		std::lock_guard<decltype(this->mutex) > guard(this->mutex);
		if (this->src != this->srcInUse) {
			this->srcInUse = this->src;
		}
	}
	if (!this->srcInUse) {
		for (auto& b : buf) {
			for (auto& c : b.channel) {
				c = 0;
			}
		}
		return false;
	}
	return this->srcInUse->fillSampleBuffer(buf);
}
