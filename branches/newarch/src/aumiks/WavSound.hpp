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

// Home page: http://aumiks.googlecode.com

/**
 * @author Ivan Gagis <igagis@gmail.com>
 */


#pragma once


#include <ting/Array.hpp>
#include <ting/fs/File.hpp>

#include "Sound.hpp"



namespace aumiks{



class WavSound : public aumiks::Sound{
	
	ting::u8 chans;
	ting::u32 freq;
	
protected:
	WavSound(ting::u8 chans, ting::s32 freq) :
			chans(chans),
			freq(freq)
	{}

public:
	inline ting::u8 NumChannels()const throw(){
		return this->chans;
	}
	
	inline ting::u32 SamplingRate()const throw(){
		return this->freq;
	}

	//TODO:
//	inline Ref<WavSound::Channel> Play(unsigned numLoops = 1)const{
//		Ref<WavSound::Channel> ret = this->CreateWavChannel();
//		ret->Play(numLoops);
//		return ret;
//	}

	static ting::Ref<WavSound> Load(const std::string& fileName);
	static ting::Ref<WavSound> Load(ting::fs::File& fi);
};



}
