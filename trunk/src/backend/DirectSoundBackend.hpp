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

/* TODO: uncomment
#ifndef WIN32
#error "compiling in non-Win32 environment"
#endif
*/

#include <dsound.h>
#include <cstring>

#include "../aumiks/aumiks.hpp"
#include "../aumiks/Exc.hpp"



namespace{

class DirectSoundBackend : public aumiks::AudioBackend{
	struct DirectSound{
		LPDIRECTSOUND ds;//LP prefix means long pointer
		
		DirectSound(){
			if(DirectSoundCreate(NULL, &this->ds, NULL) != DS_OK){
				throw aumiks::Exc("DirectSound object creation failed");
			}
			
			try{
				HWND hwnd = GetDesktopWindow();
				if(hwnd == NULL){
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
		LPDIRECTSOUNDBUFFER dsb; //LP stands for long pointer
		
		DirectSoundBuffer(DirectSound& ds, unsigned bufferSizeFrames, aumiks::E_Format format){
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
			dsbdesc.dwBufferBytes = 2 * aumiks::BytesPerFrame(format) * bufferSizeFrames;
			dsbdesc.lpwfxFormat = &wf; 
			
			if(dsbdesc.dwBufferBytes < DSBSIZE_MIN || DSBSIZE_MAX < dsbdesc.dwBufferBytes){
				throw aumiks::Exc("DirectSound: requested buffer size is out of supported size range [DSBSIZE_MIN, DSBSIZE_MAX]");
			}
			
			if(ds.ds->CreateSoundBuffer(&dsbdesc, &this->dsb, NULL) != DS_OK){
				throw aumiks::Exc("DirectSound: creating sound buffer failed");
			}
		}
		
		~DirectSoundBuffer(){
			this->dsb->Release();
		}
	} dsb;
	
	
	DirectSoundBackend(unsigned bufferSizeFrames, aumiks::E_Format format) :
			dsb(this->ds, bufferSizeFrames, format)
	{}

public:

	~DirectSoundBackend(){
		//TODO:
	}
	
	inline static ting::Ptr<DirectSoundBackend> New(unsigned bufferSizeMillis, aumiks::E_Format format){
		return ting::Ptr<DirectSoundBackend>(
				new DirectSoundBackend(bufferSizeMillis, format)
			);
	}
};

}//~namespace
