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



//TODO: remove this file


#include <list>

#include <ting/config.hpp>
#include <ting/debug.hpp>
#include <ting/types.hpp>
#include <ting/Thread.hpp>
#include <ting/Array.hpp>
#include <ting/Buffer.hpp>

#include "aumiks.hpp"

#if M_OS == M_OS_WIN32 || M_OS == M_OS_WIN64
	#include "backend/DirectSoundBackend.hpp"
#elif M_OS == M_OS_LINUX
	#if defined(__ANDROID__)
		#include "backend/OpenSLESBackend.hpp"
	#else
		#include "backend/PulseAudioBackend.hpp"
//		#include "backend/ALSABackend.hpp"
	#endif
#else
	#error "Unknown OS"
#endif


using namespace aumiks;



ting::IntrusiveSingleton<Lib>::T_Instance Lib::instance;



namespace{

unsigned BufferSizeInFrames(unsigned bufferSizeMillis, E_Format format){
	//NOTE: for simplicity of conversion from lower sample rates to higher ones,
	//      the size of output buffer should be that it would hold no fractional
	//      parts of source samples when they are mixed to it.
	unsigned granularity;

	switch(format){
		case MONO_16_11025:
		case STEREO_16_11025:
			granularity = 1;
			break;
		case MONO_16_22050:
		case STEREO_16_22050:
			granularity = 2;
			break;
		case MONO_16_44100:
		case STEREO_16_44100:
			granularity = 4;
			break;
		default:
			throw aumiks::Exc("unknown sound format");
	}

	unsigned ret = aumiks::SamplingRate(format) * bufferSizeMillis / 1000;
	return ret + (granularity - ret % granularity);
}



unsigned BufferSizeInSamples(unsigned bufferSizeMillis, E_Format format){
	return BufferSizeInFrames(bufferSizeMillis, format) * aumiks::SamplesPerFrame(format);
}

}//~namespace







unsigned aumiks::BytesPerFrame(E_Format format){
	return 2 * SamplesPerFrame(format); //we only support 16 bits per sample
}


unsigned aumiks::SamplesPerFrame(E_Format format){
	switch(format){
		case aumiks::MONO_16_11025:
		case aumiks::MONO_16_22050:
		case aumiks::MONO_16_44100:
			return 1;
		case aumiks::STEREO_16_11025:
		case aumiks::STEREO_16_22050:
		case aumiks::STEREO_16_44100:
			return 2;
		default:
			throw aumiks::Exc("Unknown sound output format");
	}
}

unsigned aumiks::SamplingRate(E_Format format){
	switch(format){
		case aumiks::MONO_16_11025:
		case aumiks::STEREO_16_11025:
			return 11025;
		case aumiks::MONO_16_22050:
		case aumiks::STEREO_16_22050:
			return 22050;
		case aumiks::MONO_16_44100:
		case aumiks::STEREO_16_44100:
			return 44100;
		default:
			throw aumiks::Exc("Unknown sound output format");
	}
}



