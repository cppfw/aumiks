/* The MIT License:

Copyright (c) 2009-2012 Ivan Gagis

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



#include <pulse/simple.h>
#include <pulse/error.h>

#include "WriteBasedBackend.hpp"


namespace{

class PulseAudioBackend : public WriteBasedBackend{
	pa_simple *handle;

	PulseAudioBackend(unsigned bufferSizeFrames, aumiks::E_Format format) :
			WriteBasedBackend(bufferSizeFrames * aumiks::BytesPerFrame(format))
	{
		TRACE(<< "opening device" << std::endl)

		pa_sample_spec ss;
		ss.format = PA_SAMPLE_S16NE;//Native endian
		ss.channels = aumiks::SamplesPerFrame(format);
		ss.rate = aumiks::SamplingRate(format);

		unsigned bufferSizeInBytes = bufferSizeFrames * aumiks::BytesPerFrame(format);
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
		
		this->Start();//start thread
	}
	
	//override
	void Write(const ting::Buffer<ting::u8>& buf){
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

public:

	virtual ~PulseAudioBackend()throw(){
		this->StopThread();
		
		ASSERT(this->handle)
		pa_simple_free(this->handle);
	}
	
	inline static ting::Ptr<PulseAudioBackend> New(unsigned bufferSizeFrames, aumiks::E_Format format){
		return ting::Ptr<PulseAudioBackend>(
				new PulseAudioBackend(bufferSizeFrames, format)
			);
	}
};

}//~namespace
