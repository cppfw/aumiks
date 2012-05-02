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


#include <ting/Array.hpp>
#include <ting/fs/File.hpp>

#include "aumiks.hpp"



namespace aumiks{

using namespace ting;



class WavSound : public aumiks::Sound{
	
protected:
	WavSound(){}
	
public:
	class Channel : public aumiks::Channel{
	protected:
		ting::Inited<unsigned, 1> numLoops;//0 means loop infinitely
		
		ting::Inited<unsigned, 0> curPos;//current index in samples into sound data buffer
		
	public:
		/**
		 * @brief play channel
         * @param numLoops - number of time the sound should be repeated. 0 means repeat infinitely.
         */
		inline void Play(unsigned numLoops = 1){
			this->numLoops = numLoops;//not protected by mutex, but should not cause any serious problems
			this->aumiks::Channel::Play();
		}
	};

public:
	virtual Ref<WavSound::Channel> CreateWavChannel()const = 0;

	inline Ref<WavSound::Channel> Play(unsigned numLoops = 1)const{
		Ref<WavSound::Channel> ret = this->CreateWavChannel();
		ret->Play(numLoops);
		return ret;
	}

	static Ref<WavSound> LoadWAV(const std::string& fileName);
	static Ref<WavSound> LoadWAV(ting::fs::File& fi);
	
	
	//override
	virtual Ref<aumiks::Channel> CreateChannel()const{
		return this->CreateWavChannel();
	}
};



}
