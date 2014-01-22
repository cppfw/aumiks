/* The MIT License:

Copyright (c) 2009-2014 Ivan Gagis

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



#include <pulse/simple.h>
#include <pulse/error.h>

#include "WriteBasedBackend.hpp"


namespace{

class PulseAudioBackend : public WriteBasedBackend{
	pa_simple *handle;
	
	//override
	virtual void Write(const ting::Buffer<ting::u8>& buf){
//		ASSERT(buf.Size() == this->BufferSizeInBytes())

		if(pa_simple_write(
				this->handle,
				buf.Begin(),
				size_t(buf.SizeInBytes()),
				0 // no error return
			) < 0)
		{
			//TODO: handle error somehow, throw exception
			//ignore error
			TRACE(<< "pa_simple_write(): error" << std::endl)
		}
	}

	//override
	virtual void SetPaused(bool pause){
		//TODO:
	}
	
	//override
	virtual void Start(){
		this->StartThread();
	}
	
public:
	PulseAudioBackend(
			audout::AudioFormat outputFormat,
			ting::u32 bufferSizeFrames,
			audout::PlayerListener* listener
			
		) :
			WriteBasedBackend(listener, bufferSizeFrames * outputFormat.frame.NumChannels() * 2)//2 bytes per sample, i.e. 16 bit
	{
		TRACE(<< "opening device" << std::endl)

		pa_sample_spec ss;
		ss.format = PA_SAMPLE_S16NE;//Native endian
		ss.channels = outputFormat.frame.NumChannels();
		ss.rate = outputFormat.samplingRate.Frequency();

		unsigned bufferSizeInBytes = bufferSizeFrames * aumiks::BytesPerOutputFrame(outputFormat.frame.NumChannels());
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
				"audout", // Our application's name.
				PA_STREAM_PLAYBACK,
				0, // Use the default device.
				"sound stream", // Description of our stream.
				&ss, // our sample format.
				&cm, // channel map
				&ba, // buffering attributes.
				&error
			);

		if(!this->handle){
			TRACE(<< "error opening PulseAudio connection (" << pa_strerror(error) << ")" << std::endl)
			//TODO: more informative error message
			throw ting::Exc("error opening PulseAudio connection");
		}
	}
	
	virtual ~PulseAudioBackend()throw(){
		this->StopThread();
		
		ASSERT(this->handle)
		pa_simple_free(this->handle);
	}
};

}//~namespace
