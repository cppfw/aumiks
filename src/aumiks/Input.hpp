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
	template <ting::u8 num_channels> friend class ChanInput;
	
	ting::u8 numChannels;
	
	Input(ting::u8 numChannels) :
			numChannels(numChannels)
	{}
	
	ting::Ref<aumiks::Source> src;
	
	ting::atomic::SpinLock spinLock;
public:
	virtual ~Input()throw(){}
	
	ting::u8 NumChannels()const throw(){
		return this->numChannels;
	}
	
	void Disconnect()throw(){
		//To minimize the time with locked spinlock need to avoid object destruction
		//within the locked spinlock period. To achieve that use temporary strong reference.
		ting::Ref<aumiks::Source> tmp;
		
		if(this->src.IsValid()){
			ting::atomic::SpinLock::Guard guard(this->spinLock);
			tmp = this->src;
			tmp->isConnected = false;
			this->src.Reset();
		}
	}
	
	void Connect(const ting::Ref<aumiks::Source>& source){
		ASSERT(source.IsValid())
		ASSERT(this->NumChannels() == source->NumChannels())
		
		if(this->IsConnected()){
			throw aumiks::Exc("Input already connected");
		}
		
		if(source->IsConnected()){
			throw aumiks::Exc("Source is already connected");
		}
		
		{
			ting::atomic::SpinLock::Guard guard(this->spinLock);
			source->isConnected = true;
			this->src = source;
		}
	}
	
	//thread safe
	bool IsConnected()const{
		return this->src.IsValid();
	}
};



template <ting::u8 num_channels> class ChanInput : public Input{
	ting::Ref<aumiks::ChanSource<num_channels> > srcInUse;
public:
	
	ChanInput() :
			Input(num_channels)
	{}
	
	bool FillSampleBuffer(const ting::Buffer<ting::s32>& buf)throw(){
		if(this->src != this->srcInUse){
			ting::atomic::SpinLock::Guard guard(this->spinLock);
//			ASSERT(this->src.IsNotValid() || this->src->NumChannels() == num_channels)
			typedef aumiks::ChanSource<num_channels> T_ChanneledSource;
			this->srcInUse = this->src.template StaticCast<T_ChanneledSource>();
		}
		if(this->srcInUse.IsNotValid()){
			return false;
		}
		return this->srcInUse->FillSampleBuffer(buf);
	}
};


}
