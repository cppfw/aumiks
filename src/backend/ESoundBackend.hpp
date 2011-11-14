/* The MIT License:

Copyright (c) 2009 Ivan Gagis

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

/*
 * aumiks 0.1
 *
 * File:   ESoundBackend.hpp
 * Author: ivan
 *
 * Created on March 9, 2009, 4:10 PM
 */

#ifndef M_ESoundBackend_hpp
#define	M_ESoundBackend_hpp

#include <ting/types.hpp>
#include <ting/Buffer.hpp>
#include <ting/debug.hpp>

#include <esd.h>

#include "AudioBackend.hpp"

namespace{

class ESoundBackend : public AudioBackend{
	int handle;
public:
	ESoundBackend(unsigned requestedBufferSizeInFrames){
		this->handle = esd_play_stream_fallback(
				ESD_BITS16 | ESD_STEREO | ESD_STREAM | ESD_PLAY,
				44100,
				0,
				0
			);

		if(this->handle <= 0){
			TRACE(<< "ESound: unable to open pcm device" << std::endl)
			throw aumiks::Exc("ESound: unable to open pcm device");
		}

		this->bufSizeInFrames = 16;
		this->numChannels = 2;
		this->sampleSizeInBytes = 2;
	}

	~ESoundBackend(){
		close(this->handle);
	}

	//override
	void Write(const ting::Buffer<ting::u8>& buf){
		ASSERT(buf.SizeInBytes() % this->FrameSizeInBytes() == 0)
		write(this->handle, reinterpret_cast<const void*>(&buf[0]), buf.SizeInBytes());
	}
};

}//~namespace
#endif //~once
