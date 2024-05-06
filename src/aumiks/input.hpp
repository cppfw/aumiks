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

#pragma once

#include <utki/spin_lock.hpp>

#include <type_traits>
#include <mutex>

#include "config.hpp"
#include "source.hpp"
#include "frame.hpp"

namespace aumiks{

class input{
protected:
	std::shared_ptr<source> src;
	
	std::shared_ptr<source> src_in_use;
	
	utki::spin_lock mutex;
	
public:
	input() = default;
	
	virtual ~input() = default;
	
	void disconnect()noexcept;
	
	void connect(std::shared_ptr<aumiks::source> source);
	
	bool is_connected()const{
		return this->src.get() != nullptr;
	}

	bool fill_sample_buffer(utki::span<frame> buf)noexcept;	
};

}
