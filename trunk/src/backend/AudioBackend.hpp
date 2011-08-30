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
 * File:   AudioBackend.hpp
 * Author: ivan
 *
 * Created on March 9, 2009, 3:16 PM
 */

#pragma once

#include <ting/types.hpp>
#include <ting/Buffer.hpp>

namespace aumiks{

//base class for audio backends
class AudioBackend{
protected:
	unsigned bufSizeInFrames;
	unsigned sampleSizeInBytes;
	unsigned numChannels;
public:

	virtual ~AudioBackend(){};

	inline unsigned BufferSizeInFrames()const{
		return this->bufSizeInFrames;
	}

	inline unsigned BufferSizeInSamples()const{
		return this->bufSizeInFrames * this->numChannels;
	}

	inline unsigned BufferSizeInBytes()const{
		return this->BufferSizeInFrames() * this->FrameSizeInBytes();
	}

	inline unsigned SampleSizeInBytes()const{
		return this->sampleSizeInBytes;
	}

	inline unsigned FrameSizeInBytes()const{
		return this->SampleSizeInBytes() * this->numChannels;
	}

	virtual void Write(const ting::Buffer<ting::u8>& buf) = 0;
};

}//~namespace
