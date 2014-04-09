/* The MIT License:

Copyright (c) 2014 Ivan Gagis

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE. */

// Home page: http://aumiks.googlecode.com

/**
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once

#include "Input.hpp"
#include "Exc.hpp"


namespace aumiks{

//TODO: doxygen
class Sink{
	ting::u8 numChannels;
	ting::u32 frequency;
	
protected:
	Sink(ting::u8 numChannles, ting::u32 frequency) :
			numChannels(numChannles),
			frequency(frequency)
	{}
public:
	
	ting::u8 NumChannels()const throw(){
		return this->numChannels;
	}
	
	ting::u32 Frequency()const throw(){
		return this->frequency;
	}
	
	virtual ~Sink()throw(){}
	
	virtual void Start() = 0;
	
	virtual void Stop(){
		throw aumiks::Exc("Sink::Stop(): unsupported");
	}
};



template <ting::u8 num_channels> class ChanSink : public Sink{
protected:
	ChanSink(ting::u32 frequency) :
			Sink(num_channels, frequency)
	{}
public:
	aumiks::ChanInput<num_channels> input;
};

}
