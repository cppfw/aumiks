/* The MIT License:

Copyright (c) 2011-2012 Ivan Gagis

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

#include <ting/Thread.hpp>
#include <ting/Array.hpp>


#include "../aumiks.hpp"
#include "../Exc.hpp"



namespace{

class WriteBasedBackend : public aumiks::AudioBackend, public ting::MsgThread{
	ting::Array<ting::u8> playBuf;
protected:
	WriteBasedBackend(size_t playBufSizeInBytes) :
			playBuf(playBufSizeInBytes)
	{}
	
	inline void StopThread()throw(){
		this->PushPreallocatedQuitMessage();
		this->Join();
	}
	
	virtual void Write(const ting::Buffer<ting::u8>& buf) = 0;
	
public:
	virtual ~WriteBasedBackend()throw(){}
	
private:
	
	//override
	void Run(){
		while(!this->quitFlag){
//			TRACE(<< "Backend loop" << std::endl)
			while(ting::Ptr<ting::Message> m = this->queue.PeekMsg()){
				m->Handle();
			}

			this->FillPlayBuf(this->playBuf);
			
			//call virtual Write() function
			try{
				this->Write(this->playBuf);
			}catch(aumiks::Exc& e){
				ASSERT_INFO(false, e.What())
				return;//exit thread
			}
		}//~while
	}
};

}//~namespace
