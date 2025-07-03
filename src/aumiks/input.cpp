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

#include "input.hpp"

using namespace aumiks;

void input::connect(std::shared_ptr<aumiks::source> source)
{
	ASSERT(source)

	if (this->is_connected()) {
		throw std::logic_error("the input already connected");
	}

	if (source->is_connected()) {
		throw std::logic_error("the source is already connected");
	}

	{
		std::lock_guard<decltype(this->mutex)> guard(this->mutex);
		source->is_source_connected = true;
		this->src = std::move(source);
	}
}

void input::disconnect() noexcept
{
	std::lock_guard<decltype(this->mutex)> guard(this->mutex);

	if (this->src) {
		this->src->is_source_connected = false;
		this->src.reset();
	}
}

bool input::fill_sample_buffer(utki::span<frame> buf) noexcept
{
	{
		std::lock_guard<decltype(this->mutex)> guard(this->mutex);
		if (this->src != this->src_in_use) {
			this->src_in_use = this->src;
		}
	}
	if (!this->src_in_use) {
		for (auto& b : buf) {
			for (auto& c : b.channel) {
				c = 0;
			}
		}
		return false;
	}
	return this->src_in_use->fill_sample_buffer(buf);
}
