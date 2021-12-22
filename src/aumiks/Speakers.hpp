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

#include <audout/player.hpp>

#include "Sink.hpp"

namespace aumiks{

//TODO: make singleton
class Speakers :
		public aumiks::Sink,
		private audout::listener
{
	std::vector<frame> smpBuf;

	// this function is not thread-safe, but it is supposed to be called from special audio thread
	void fill(utki::span<std::int16_t> playBuf)noexcept override;
	
public:
	const std::uint32_t samplingRate;
	
private:
	audout::player player;
	
public:
	Speakers(audout::rate samplingRate, uint16_t bufferSizeMillis = 100);

	Speakers(const Speakers&) = delete;
	Speakers& operator=(const Speakers&) = delete;
	
	void start()override;

	void stop()override;
};

}
