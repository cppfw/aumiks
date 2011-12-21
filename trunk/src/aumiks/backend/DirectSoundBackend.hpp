/* The MIT License:

Copyright (c) 2011 Ivan Gagis

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

/**
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once


#ifndef WIN32
#error "compiling in non-Win32 environment"
#endif


#include <cstring>

#include <initguid.h> //The header file initguid.h is required to avoid the error message "undefined reference to `IID_IDirectSoundBuffer8'".
#include <dsound.h>

#include <ting/WaitSet.hpp>
#include <ting/Thread.hpp>

#include "../aumiks.hpp"
#include "../Exc.hpp"



namespace{

class WinEvent : public ting::Waitable{
	HANDLE eventForWaitable;

	ting::u32 flagsMask;//flags to wait for

	//override
	virtual void SetWaitingEvents(ting::u32 flagsToWaitFor){
		//Only possible flag values are READ and 0 (NOT_READY)
		if(flagsToWaitFor != 0 && flagsToWaitFor != ting::Waitable::READ){
			ASSERT_INFO(false, "flagsToWaitFor = " << flagsToWaitFor)
			throw ting::Exc("WinEvent::SetWaitingEvents(): flagsToWaitFor should be ting::Waitable::READ or 0, other values are not allowed");
		}

		this->flagsMask = flagsToWaitFor;
	}

	//returns true if signaled
	//override
	virtual bool CheckSignalled(){
		switch(WaitForSingleObject(this->eventForWaitable, 0)){
			case WAIT_OBJECT_0: //event is signaled
				this->SetCanReadFlag();
				if(ResetEvent(this->eventForWaitable) == 0){
					ASSERT(false)
					throw ting::Exc("WinEvent::Reset(): ResetEvent() failed");
				}
				break;
			case WAIT_TIMEOUT: //event is not signalled
				this->ClearCanReadFlag();
				break;
			default:
				throw ting::Exc("WinEvent: error when checking event state, WaitForSingleObject() failed");
		}
		
		return (this->readinessFlags & this->flagsMask) != 0;
	}
public:
	//override
	HANDLE GetHandle(){
		return this->eventForWaitable;
	}
	
	WinEvent(){
		this->eventForWaitable = CreateEvent(
			0, //security attributes
			TRUE, //manual-reset
			FALSE, //not signalled initially
			0 //no name
		);
		if(this->eventForWaitable == 0){
			throw ting::Exc("WinEvent::WinEvent(): could not create event (Win32) for implementing Waitable");
		}
	}
	
	~WinEvent(){
		CloseHandle(this->eventForWaitable);
	}
};



class DirectSoundBackend : public aumiks::AudioBackend, public ting::MsgThread{
	struct DirectSound{
		LPDIRECTSOUND8 ds;//LP prefix means long pointer
		
		DirectSound(){
			if(DirectSoundCreate8(0, &this->ds, 0) != DS_OK){
				throw aumiks::Exc("DirectSound object creation failed");
			}
			
			try{
				HWND hwnd = GetDesktopWindow();
				if(hwnd == 0){
					throw aumiks::Exc("DirectSound: no foreground window found");
				}

				if(this->ds->SetCooperativeLevel(hwnd, DSSCL_PRIORITY) != DS_OK){
					throw aumiks::Exc("DirectSound: setting cooperative level failed");
				}
			}catch(...){
				this->ds->Release();
				throw;
			}
		}
		~DirectSound(){
			this->ds->Release();
		}
	} ds;

	struct DirectSoundBuffer{
		LPDIRECTSOUNDBUFFER8 dsb; //LP stands for long pointer
		
		unsigned halfSize;
		
		DirectSoundBuffer(DirectSound& ds, unsigned bufferSizeFrames, aumiks::E_Format format) :
				halfSize(aumiks::BytesPerFrame(format) * bufferSizeFrames)
		{
			WAVEFORMATEX wf;
			memset(&wf, 0, sizeof(WAVEFORMATEX));

			wf.nChannels = aumiks::SamplesPerFrame(format);
			wf.nSamplesPerSec = aumiks::SamplingRate(format);

			wf.wFormatTag = WAVE_FORMAT_PCM;
			wf.wBitsPerSample = 16;
			wf.nBlockAlign = wf.nChannels * (wf.wBitsPerSample / 8);
			wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;
			
			DSBUFFERDESC dsbdesc;
			memset(&dsbdesc, 0, sizeof(DSBUFFERDESC)); 
			
			dsbdesc.dwSize = sizeof(DSBUFFERDESC); 
			dsbdesc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GLOBALFOCUS; 
			dsbdesc.dwBufferBytes = 2 * this->halfSize;
			dsbdesc.lpwfxFormat = &wf; 
			
			if(dsbdesc.dwBufferBytes < DSBSIZE_MIN || DSBSIZE_MAX < dsbdesc.dwBufferBytes){
				throw aumiks::Exc("DirectSound: requested buffer size is out of supported size range [DSBSIZE_MIN, DSBSIZE_MAX]");
			}
			
			{
				LPDIRECTSOUNDBUFFER dsb1;
				if(ds.ds->CreateSoundBuffer(&dsbdesc, &dsb1, 0) != DS_OK){
					throw aumiks::Exc("DirectSound: creating sound buffer failed");
				}
				if(dsb1->QueryInterface(IID_IDirectSoundBuffer8, (LPVOID*)&this->dsb) != DS_OK){
					dsb1->Release();
					throw aumiks::Exc("DirectSound: querying sound buffer interface failed");
				}
				dsb1->Release();
			}
			
			//init buffer with silence, i.e. fill it with 0'es
			{
				LPVOID addr;
				DWORD size;
				
				//lock the entire buffer
				if(this->dsb->Lock(
						0,
						0, //ignored because of the DSBLOCK_ENTIREBUFFER flag
						&addr,
						&size,
						0, //wraparound not needed
						0, //size of wraparound not needed
						DSBLOCK_ENTIREBUFFER
					) != DS_OK)
				{
					this->dsb->Release();
					throw aumiks::Exc("DirectSound: locking buffer failed");
				}
				
				ASSERT(addr != 0)
				ASSERT(size == 2 * this->halfSize)
				
				//set buffer to 0'es
				memset(addr, 0, size);
				
				//unlock the buffer
				if(this->dsb->Unlock(addr, size, 0, 0) != DS_OK){
					this->dsb->Release();
					throw aumiks::Exc("DirectSound: unlocking buffer failed");
				}
			}
			
			this->dsb->SetCurrentPosition(0);
		}
		
		~DirectSoundBuffer(){
			this->dsb->Release();
		}
	} dsb;
	
	WinEvent event1, event2;
	
	DirectSoundBackend(unsigned bufferSizeFrames, aumiks::E_Format format) :
			dsb(this->ds, bufferSizeFrames, format)
	{
		//Set notification points
		{
			LPDIRECTSOUNDNOTIFY8 notify;
			
			//Get IID_IDirectSoundNotify interface
			if(this->dsb.dsb->QueryInterface(
					IID_IDirectSoundNotify8,
					(LPVOID*)&notify
				) != DS_OK)
			{
				throw aumiks::Exc("DirectSound: obtaining IID_IDirectSoundNotify interface failed");
			}
			
			ting::StaticBuffer<DSBPOSITIONNOTIFY, 2> pos;
			pos[0].dwOffset = 0;
			pos[0].hEventNotify = this->event1.GetHandle();
			pos[1].dwOffset = this->dsb.halfSize;
			pos[1].hEventNotify = this->event2.GetHandle();
			
			if(notify->SetNotificationPositions(pos.Size(), pos.Begin()) != DS_OK){
				notify->Release();
				throw aumiks::Exc("DirectSound: setting notification positions failed");
			}
			
			//release IID_IDirectSoundNotify interface
			notify->Release();
		}
		
		//start playing thread
		this->Start();
		
		//launch buffer playing
		if(this->dsb.dsb->Play(
				0, //reserved, must be 0
				0,
				DSBPLAY_LOOPING
			) != DS_OK)
		{
			throw aumiks::Exc("DirectSound: failed to play buffer, Play() method failed");
		}
	}
	
	inline void FillDSBuffer(unsigned partNum){
		ASSERT(partNum == 0 || partNum == 1)
		LPVOID addr;
		DWORD size;

		//lock the second part of buffer
		if(this->dsb.dsb->Lock(
				this->dsb.halfSize * partNum, //offset
				this->dsb.halfSize, //size
				&addr,
				&size,
				0, //wraparound not needed
				0, //size of wraparound not needed
				0 //no flags
			) != DS_OK)
		{
			TRACE(<< "DirectSound thread: locking buffer failed" << std::endl)
			return;
		}

		ASSERT(addr != 0)
		ASSERT(size == this->dsb.halfSize)

		ting::Buffer<ting::u8> buf(static_cast<ting::u8*>(addr), size);
		this->FillPlayBuf(buf);

		//unlock the buffer
		if(this->dsb.dsb->Unlock(addr, size, 0, 0) != DS_OK){
			TRACE(<< "DirectSound thread: unlocking buffer failed" << std::endl)
			ASSERT(false)
		}
	}
	
	//override
	void Run(){
		ting::WaitSet ws(3);
		
		ws.Add(&this->queue, ting::Waitable::READ);
		ws.Add(&this->event1, ting::Waitable::READ);
		ws.Add(&this->event2, ting::Waitable::READ);
		
		while(!this->quitFlag){
//			TRACE(<< "Backend loop" << std::endl)
			
			ws.Wait();
			
			if(this->queue.CanRead()){
				while(ting::Ptr<ting::Message> m = this->queue.PeekMsg()){
					m->Handle();
				}
			}

			//if first buffer playing has started, then fill the second one
			if(this->event1.CanRead()){
				this->FillDSBuffer(1);
			}
			
			//if second buffer playing has started, then fill the first one
			if(this->event2.CanRead()){
				this->FillDSBuffer(0);
			}
		}//~while
		
		ws.Remove(&this->event2);
		ws.Remove(&this->event1);
		ws.Remove(&this->queue);
	}

public:
	~DirectSoundBackend(){
		//stop buffer playing
		if(this->dsb.dsb->Stop() != DS_OK){
			ASSERT(false)
		}
		
		//Stop playing thread
		this->PushQuitMessage();
		this->Join();
	}
	
	inline static ting::Ptr<DirectSoundBackend> New(unsigned bufferSizeMillis, aumiks::E_Format format){
		return ting::Ptr<DirectSoundBackend>(
				new DirectSoundBackend(bufferSizeMillis, format)
			);
	}
};

}//~namespace
