/* The MIT License:

Copyright (c) 2012 Ivan Gagis

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



#include "Channel.hpp"



using namespace aumiks;



bool Channel::FillSmpBufAndApplyEffects(ting::Buffer<ting::s32>& buf, unsigned freq, unsigned chans){
	if(this->stopFlag){
		return true;
	}
	
	if(!this->soundStopped){
		this->soundStopped = this->FillSmpBuf(buf, freq, chans);
	}else{
		//clear smp buffer
		memset(buf.Begin(), 0, buf.SizeInBytes());
	}

	//TODO:
	//Call channel effects.
	//If there are no any effects, it should return ch->soundStopped, otherwise, depending on the effects.
//	bool ret = this->ApplyEffectsToSmpBuf(ch);

	//TODO:
//	if(this->isMuted){
//		return ret;
//	}

	return this->soundStopped;//TODO: what to return?
}


