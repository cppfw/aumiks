/* The MIT License:

Copyright (c) 2011-2014 Ivan Gagis

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

// Home page: http://audout.googlecode.com

/**
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once

#include <ting/mt/MsgThread.hpp>
#include <ting/Array.hpp>

#include "../Player.hpp"
#include "../PlayerListener.hpp"


namespace{

class WriteBasedBackend : public audout::Player, private ting::mt::MsgThread{
	ting::Array<ting::s16> playBuf;
protected:
	ting::Inited<bool, true> isPaused;
	
	WriteBasedBackend(
			audout::PlayerListener* listener,
			size_t playBufSizeInSamples
		) :
			audout::Player(listener),
			playBuf(playBufSizeInSamples)
	{}
	
	inline void StopThread()throw(){
		this->PushPreallocatedQuitMessage();
		this->Join();
	}
	
	inline void StartThread(){
		this->Thread::Start();
	}
	
	virtual void Write(const ting::Buffer<ting::s16>& buf) = 0;
	
public:
	virtual ~WriteBasedBackend()throw(){}
	
private:
	
	//override
	void Run(){
		while(!this->quitFlag){
//			TRACE(<< "Backend loop" << std::endl)
			
			if(this->isPaused){
				this->queue.GetMsg()->Handle();
				continue;
			}
			
			while(ting::Ptr<ting::mt::Message> m = this->queue.PeekMsg()){
				m->Handle();
			}

			this->Listener()->FillPlayBuf(this->playBuf);
			
			this->Write(this->playBuf);
		}//~while
	}
	
	class SetPausedMessage : public ting::mt::Message{
		WriteBasedBackend &wbe;
		bool pause;
	public:
		SetPausedMessage(WriteBasedBackend &wbe, bool pause) :
				wbe(wbe),
				pause(pause)
		{}
		
		//override
		void Handle(){
			this->wbe.isPaused = this->pause;
		}
	};
	
	//override
	virtual void SetPaused(bool pause){
		this->PushMessage(ting::Ptr<ting::mt::Message>(
				new SetPausedMessage(*this, pause)
			));
	}
};

}//~namespace
