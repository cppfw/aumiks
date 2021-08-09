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

#include <utki/spin_lock.hpp>

#include <type_traits>
#include <mutex>

#include "config.hpp"
#include "Exc.hpp"
#include "Source.hpp"
#include "Frame.hpp"

namespace aumiks{

class Input{
protected:
	std::shared_ptr<Source> src;
	
	std::shared_ptr<Source> srcInUse;
	
	utki::spin_lock mutex;
	
public:
	Input(){}
	
	virtual ~Input()noexcept{}
	
	void disconnect()noexcept;
	
	void connect(std::shared_ptr<Source> source);
	
	bool isConnected()const{
		return this->src.get() != nullptr;
	}

	bool fillSampleBuffer(utki::span<Frame> buf)noexcept;	
};

}
