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


#include <ting/Array.hpp>
#include <ting/File.hpp>

#include "aumiks.hpp"



namespace aumiks{

using namespace ting;



class WavSound : public aumiks::Sound{
	
protected:
	WavSound(){}
	
public:
	class Channel : public aumiks::Channel{

	};

public:
	virtual Ref<WavSound::Channel> CreateChannel()const = 0;

	inline Ref<WavSound::Channel> Play(u8 volume = u8(-1))const{
		Ref<WavSound::Channel> ret = this->CreateChannel();
		ret->SetVolume(volume);
		ret->Play();
		return ret;
	}

	static Ref<WavSound> LoadWAV(const std::string& fileName);
	static Ref<WavSound> LoadWAV(File& fi);
};



}
