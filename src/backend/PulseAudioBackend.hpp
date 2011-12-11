/* The MIT License:

Copyright (c) 2009-2011 Ivan Gagis

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

// Homepage: http://aumiks.googlecode.com

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
		
		switch(format){
			case aumiks::MONO_16_11025:
				TRACE(<< "Requested format: Mono 11025" << std::endl)
				ss.channels = 1;
				ss.rate = 11025;
				break;
			case aumiks::STEREO_16_11025:
				TRACE(<< "Requested format: Stereo 11025" << std::endl)
				ss.channels = 2;
				ss.rate = 11025;
				break;
			case aumiks::MONO_16_22050:
				TRACE(<< "Requested format: Mono 22050" << std::endl)
				ss.channels = 1;
				ss.rate = 22050;
				break;
			case aumiks::STEREO_16_22050:
				TRACE(<< "Requested format: Stereo 22050" << std::endl)
				ss.channels = 2;
				ss.rate = 22050;
				break;
			case aumiks::MONO_16_44100:
				TRACE(<< "Requested format: Mono 44100" << std::endl)
				ss.channels = 1;
				ss.rate = 44100;
				break;
			case aumiks::STEREO_16_44100:
				TRACE(<< "Requested format: Stereo 44100" << std::endl)
				ss.channels = 2;
				ss.rate = 44100;
				break;
			default:
				throw aumiks::Exc("unknown sound output format requested");
		}

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

	~PulseAudioBackend(){
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
