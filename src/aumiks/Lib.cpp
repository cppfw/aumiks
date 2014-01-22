/* The MIT License:

Copyright (c) 2012-2014 Ivan Gagis

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


#include <ting/config.hpp>
#include <algorithm>

#include "Lib.hpp"


using namespace aumiks;



ting::IntrusiveSingleton<Lib>::T_Instance Lib::instance;



Lib::Lib(audout::AudioFormat outputFormat, ting::u16 bufferSizeMillis) :
		addList(&actionsList1),
		handleList(&actionsList2),
		outputFormat(outputFormat),
		bufSizeInFrames(outputFormat.samplingRate.Frequency() * bufferSizeMillis / 1000),
		masterChannel(aumiks::MixChannel::New(true)),
		smpBuf(bufSizeInFrames * outputFormat.frame.NumChannels())
{
	//backend must be initialized after all the essential parts of aumiks are initialized,
	//because after the backend object is created, it starts calling the FillPlayBuf() method periodically.
	this->backend->Start();
}



Lib::~Lib()throw(){
	delete reinterpret_cast<AudioBackend*>(this->backend);
}



void aumiks::Lib::FillPlayBuf(ting::Buffer<ting::u8>& playBuf){
	ASSERT(playBuf.Size() == this->smpBuf.Size() * 2)
	
	//handle actions if any
	
	{
		//swap actions lists
		ting::atomic::SpinLock::Guard spinlockGuard(this->actionsSpinLock);
		std::swap(this->addList, this->handleList);
	}

	//handle actions
	while(this->handleList->size()){
		this->handleList->back()->Perform();
		this->handleList->pop_back();
	}
	
	//mix channels to smpBuf
	this->masterChannel->FillSmpBufAndApplyEffects(this->smpBuf, this->outputFormat.samplingRate.Frequency(), this->outputFormat.frame.NumChannels());

//	TRACE(<< "mixed, copying to playbuf..." << std::endl)
//	TRACE(<< "this->smpBuf = " << this->smpBuf << std::endl)

	this->CopySmpBufToPlayBuf(playBuf);
}



void aumiks::Lib::CopySmpBufToPlayBuf(ting::Buffer<ting::u8>& playBuf){
	//playBuf size is in bytes and we have 2 bytes per sample always.
	ASSERT((this->smpBuf.Size() * 2) == playBuf.Size())

	const ting::s32 *src = this->smpBuf.Begin();
	ting::u8* dst = playBuf.Begin();
	for(; src != this->smpBuf.End(); ++src){
		ting::s32 tmp = *src;
		ting::util::ClampTop(tmp, 0x7fff);
		ting::util::ClampBottom(tmp, -0x7fff);

		ASSERT(playBuf.Begin() <= dst && dst <= playBuf.End() - 1)
		*dst = ting::u8(tmp);
		++dst;
		ASSERT(playBuf.Begin() <= dst && dst <= playBuf.End() - 1)
		*dst = ting::u8(tmp >> 8);
		++dst;
	}
	ASSERT(dst == playBuf.End())
}
