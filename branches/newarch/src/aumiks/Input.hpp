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

template <ting::u8 num_channels> class Input{
	ting::Ref<aumiks::ChanSource<num_channels> > src;
	
	ting::Ref<aumiks::ChanSource<num_channels> > srcInUse;
	
	ting::atomic::SpinLock spinLock;
public:
	//thread safe
	bool IsConnected()const{
		return this->src.IsValid();
	}
	
	void Connect(const ting::Ref<aumiks::ChanSource<num_channels> >& source){
		ASSERT(source.IsValid())
		
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
	
	void Disconnect()throw(){
		//To minimize the time with locked spinlock need to avoid object destruction
		//within the locked spinlock period. To achieve that use temporary strong reference.
		ting::Ref<aumiks::ChanSource<num_channels> > tmp;
		
		if(this->src.IsValid()){
			ting::atomic::SpinLock::Guard guard(this->spinLock);
			tmp = this->src;
			tmp->isConnected = false;
			this->src.Reset();
		}
	}
	
	bool FillSampleBuffer(const ting::Buffer<ting::s32>& buf)throw(){
		if(this->src != this->srcInUse){
			ting::atomic::SpinLock::Guard guard(this->spinLock);
			this->srcInUse = this->src;
		}
		if(this->srcInUse.IsNotValid()){
			return false;
		}
		return this->srcInUse->FillSampleBuffer(buf);
	}
};


}
