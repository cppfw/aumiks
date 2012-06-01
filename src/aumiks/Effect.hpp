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

/**
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once

#include <ting/Ref.hpp>



namespace aumiks{



class Channel;



/**
 * @brief Interface for filling the sample buffer.
 * TODO: write docs about chain of sample buffer fillers.
 */
class SampleBufferFiller{
	friend class aumiks::Channel;
	
	//pointer to the next buffer filler in the chain.
	//If 0 then this is the last one in the chain.
	ting::Inited<SampleBufferFiller*, 0> next;
	
	ting::Inited<volatile bool, false> isOff;
	
	inline bool FillSmpBufInternal(ting::Buffer<ting::s32>& buf, unsigned freq, unsigned chans){
		if(this->isOff){
			return this->FillSmpBufFromNextByChain(buf, freq, chans);
		}
		
		return this->FillSmpBuf(buf, freq, chans);
	}
	
protected:
    //TODO: re-wise docs
	/**
	 * @brief This function is called when more data to play is needed.
	 * Override this method in your Channel implementation.
	 * Depending on the selected output format (sampling rate, mono/stereo) the corresponding method is called.
	 * The return value indicates whether the sound has finished playing or not.
	 * Note, that channel playing may continue even if sound has stopped playing, this is
	 * possible if there are any effects added to this channel which keeps playing, for example
	 * an echo effect.
	 * @param buf - the sample buffer to fill with the data to play.
	 * @param freq - sampling rate in Hertz.
	 * @param chans - number of channels (1 = mono, 2 = stereo, etc.).
	 * @return true if sound playing has finished. Returning true will result in that the channel will be removed
	 *         from the pool of playing channels and the contents of the 'buf' after this call will not be played.
	 * @return false otherwise.
	 */
	virtual bool FillSmpBuf(ting::Buffer<ting::s32>& buf, unsigned freq, unsigned chans) = 0;
	
	//TODO: doxygen
	inline bool FillSmpBufFromNextByChain(ting::Buffer<ting::s32>& buf, unsigned freq, unsigned chans){
		if(!this->next){
			memset(buf.Begin(), 0, buf.SizeInBytes());
			return false;
		}
		return this->next->FillSmpBufInternal(buf, freq, chans);
	}
	
public:
	virtual ~SampleBufferFiller()throw(){}
};



/**
 * @brief Base class for effect classes which can be applied to a playing sound.
 * The effects should derive from this class and re-implement the virtual methods
 * which are called to apply the effect to a sound when it is played.
 * The effects can be added to the playing channel.
 */
class Effect : public SampleBufferFiller, public virtual ting::RefCounted{
	friend class aumiks::Channel;
	
	typedef std::list<ting::Ref<aumiks::Effect> > T_EffectsList;
	typedef T_EffectsList::iterator T_EffectsIter;

public:
};


}//~namespace
