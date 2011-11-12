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

/**
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once



#include <pulse/simple.h>
#include <pulse/error.h>

#include "../aumiks/aumiks.hpp"
#include "../aumiks/Exc.hpp"



namespace aumiks{

class PulseAudioBackend : public aumiks::Lib::AudioBackend{
	pa_simple *handle;

	PulseAudioBackend(unsigned bufferSizeFrames, E_Format format){
		//TODO: get actual buffer size from pulseaudio
		
		TRACE(<< "opening device" << std::endl)

		pa_sample_spec ss;
		ss.format = PA_SAMPLE_S16NE;//Native endian
		
		unsigned bufferSizeInBytes;
		switch(format){
			//TODO:
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
			//TODO: handle error somehow
			//ignore error
			TRACE(<< "pa_simple_write(): error" << std::endl)
        }
	}

public:

	~PulseAudioBackend(){
		ASSERT(this->handle)
		pa_simple_free(this->handle);
	}
	
	inline static ting::Ptr<PulseAudioBackend> New(unsigned bufferSizeMillis, E_Format format){
		return ting::Ptr<PulseAudioBackend>(
				new PulseAudioBackend(bufferSizeMillis, format)
			);
	}
};

}//~namespace
