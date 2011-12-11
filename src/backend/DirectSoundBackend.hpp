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

#include <dsound.h>
#include <cstring>

#include "../aumiks/aumiks.hpp"
#include "../aumiks/Exc.hpp"



namespace{

class DirectSoundBackend : public aumiks::AudioBackend{
	struct DirectSound{
		LPDIRECTSOUND8 ds;//LP prefix means long pointer
		
		DirectSound(){
			if(DirectSoundCreate8(NULL, &this->ds, NULL) != DS_OK){
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
				IDirectSound_Release(this->ds);
				throw;
			}
		}
		~DirectSound(){
			IDirectSound_Release(this->ds);
		}
	} ds;

	
	
	DirectSoundBackend(unsigned bufferSizeFrames, aumiks::E_Format format){
		//TODO: rewrite for DirectSound
		
DSBUFFERDESC bd;
  LPDIRECTSOUNDBUFFER ppdsb, psdsb;
  WAVEFORMATEX wf;

  memset (&bd, 0, sizeof (DSBUFFERDESC));
  bd.dwSize = sizeof (DSBUFFERDESC);
  bd.dwFlags = DSBCAPS_PRIMARYBUFFER;
  bd.dwBufferBytes = 0;     //must be 0 for primary buffer
  bd.lpwfxFormat = NULL;    //must be null for primary buffer

  memset (&wf, 0, sizeof (WAVEFORMATEX));
  wf.wFormatTag = WAVE_FORMAT_PCM;
  wf.nChannels = 2;
  wf.nSamplesPerSec = 44100;
  wf.wBitsPerSample = 16;
  wf.nBlockAlign = 4;
  wf.nAvgBytesPerSec = 176400;

  if (SUCCEEDED (pds->CreateSoundBuffer (&bd, &ppdsb, NULL))){
    ppdsb->SetFormat (&wf);
  }
		
		TRACE(<< "opening device" << std::endl)

		pa_sample_spec ss;
		ss.format = PA_SAMPLE_S16NE;//Native endian
		
		unsigned bufferSizeInBytes;
		switch(format){
			case aumiks::MONO_16_11025:
				TRACE(<< "Requested format: Mono 11025" << std::endl)
				ss.channels = 1;
				ss.rate = 11025;
				bufferSizeInBytes = bufferSizeFrames * 2;//2 bytes per sample, 1 sample per frame
				break;
			case aumiks::STEREO_16_11025:
				TRACE(<< "Requested format: Stereo 11025" << std::endl)
				ss.channels = 2;
				ss.rate = 11025;
				bufferSizeInBytes = bufferSizeFrames * 2 * 2;//2 bytes per sample, 2 samples per frame
				break;
			case aumiks::MONO_16_22050:
				TRACE(<< "Requested format: Mono 22050" << std::endl)
				ss.channels = 1;
				ss.rate = 22050;
				bufferSizeInBytes = bufferSizeFrames * 2;//2 bytes per sample, 1 sample per frame
				break;
			case aumiks::STEREO_16_22050:
				TRACE(<< "Requested format: Stereo 22050" << std::endl)
				ss.channels = 2;
				ss.rate = 22050;
				bufferSizeInBytes = bufferSizeFrames * 2 * 2;//2 bytes per sample, 2 samples per frame
				break;
			case aumiks::MONO_16_44100:
				TRACE(<< "Requested format: Mono 44100" << std::endl)
				ss.channels = 1;
				ss.rate = 44100;
				bufferSizeInBytes = bufferSizeFrames * 2;//2 bytes per sample, 1 sample per frame
				break;
			case aumiks::STEREO_16_44100:
				TRACE(<< "Requested format: Stereo 44100" << std::endl)
				ss.channels = 2;
				ss.rate = 44100;
				bufferSizeInBytes = bufferSizeFrames * 2 * 2;//2 bytes per sample, 2 samples per frame
				break;
			default:
				throw aumiks::Exc("unknown sound output format requested");
		}

		pa_buffer_attr ba;
		ba.fragsize = bufferSizeInBytes;
		ba.tlength = bufferSizeInBytes;
		ba.minreq = bufferSizeInBytes / 2;
		ba.maxlength = ba.tlength;
		ba.prebuf = ba.tlength;
		
		pa_channel_map cm;
		pa_channel_map_init_auto(&cm, ss.channels, PA_CHANNEL_MAP_WAVEEX);

		int error;

		this->handle = pa_simple_new(
				0, // Use the default server.
				"aumiks", // Our application's name.
				PA_STREAM_PLAYBACK,
				0, // Use the default device.
				"Music", // Description of our stream.
				&ss, // our sample format.
				&cm, // channel map
				&ba, // buffering attributes.
				&error
			);

		if(!this->handle){
			TRACE(<< "error opening PulseAudio connection (" << pa_strerror(error) << ")" << std::endl)
			throw aumiks::Exc("error opening PulseAudio connection");
		}
	}

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
