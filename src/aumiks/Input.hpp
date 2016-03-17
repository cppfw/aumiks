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

#include "Exc.hpp"
#include "Source.hpp"

namespace aumiks{



class Input{
	template <std::uint8_t> friend class ChanneledInput;
	
	std::uint8_t numChannels;
	
	Input(std::uint8_t numChannels) :
			numChannels(numChannels)
	{}
	
	std::shared_ptr<aumiks::Source> src;
	
	utki::SpinLock spinLock;
public:
	virtual ~Input()throw(){}
	
	std::uint8_t NumChannels()const throw(){
		return this->numChannels;
	}
	
	void Disconnect()throw(){
		//To minimize the time with locked spinlock need to avoid object destruction
		//within the locked spinlock period. To achieve that use temporary strong reference.
		std::shared_ptr<aumiks::Source> tmp;
		
		if(this->src){
			std::lock_guard<utki::SpinLock> guard(this->spinLock);
			tmp = this->src;
			tmp->isConnected = false;
			this->src.reset();
		}
	}
	
	void Connect(const std::shared_ptr<aumiks::Source>& source){
		ASSERT(source)
		ASSERT(this->NumChannels() == source->NumChannels())
		
		if(this->IsConnected()){
			throw aumiks::Exc("Input already connected");
		}
		
		if(source->IsConnected()){
			throw aumiks::Exc("Source is already connected");
		}
		
		{
			std::lock_guard<utki::SpinLock> guard(this->spinLock);
			source->isConnected = true;
			this->src = source;
		}
	}
	
	//thread safe
	bool IsConnected()const{
		return this->src.operator bool();
	}
};



template <std::uint8_t num_channels> class ChanneledInput : public Input{
	std::shared_ptr<aumiks::Source> srcInUse;

public:
	
	ChanneledInput() :
			Input(num_channels)
	{}
	
	bool FillSampleBuffer(utki::Buf<std::int32_t> buf)noexcept{
		if(this->src != this->srcInUse){
			std::lock_guard<utki::SpinLock> guard(this->spinLock);
			ASSERT(!this->src || this->src->NumChannels() == num_channels)
			this->srcInUse = this->src;//this->src.template StaticCast<T_ChanneledSource>();
		}
		if(!this->srcInUse){
			return false;
		}
		typedef aumiks::ChanneledSource<num_channels> T_ChanneledOutput;
		return static_cast<T_ChanneledOutput&>(*this->srcInUse).fillSampleBuffer(buf);
	}
};


}
