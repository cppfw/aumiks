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


#include <list>

#include <ting/Array.hpp>
#include <ting/Ref.hpp>

#include "Channel.hpp"



namespace aumiks{



class MixChannel : public aumiks::Channel{
	
	typedef std::list<ting::Ref<aumiks::Channel> > T_ChannelList;
	typedef T_ChannelList::iterator T_ChannelIter;
	T_ChannelList channels;//should be accessed from audio thread only
	
	//TODO: assign buffer in the audio thread when channel starts to play
	ting::Array<ting::s32> smpBuf;
	
	bool isPersistent;
	
	MixChannel(bool isPersistent = false) :
			isPersistent(isPersistent)
	{}
	
	
	//override
	bool FillSmpBuf(ting::Buffer<ting::s32>& buf, unsigned freq, unsigned chans);
	
	
	void MixSmpBufTo(ting::Buffer<ting::s32>& buf);
	
public:
	virtual ~MixChannel()throw(){}
	
	//TODO: doxygen
	//playing the same channel the second time results in undefined behavior
	void PlayChannel_ts(const ting::Ref<aumiks::Channel>& channel);
	
	/**
	 * @brief Create a new MixChannel object.
     * @param isPersistent - true = the channel will continue to be in the playing state even
	 *                       after all the child channels have finished playing.
	 *                       flase = the channel will stop as fast as all the child channels has finished playing.
     * @return a reference to a newly creted object.
     */
	static inline ting::Ref<aumiks::MixChannel> New(bool isPersistent = false){
		return ting::Ref<aumiks::MixChannel>(
				new aumiks::MixChannel(isPersistent)
			);
	}
};


}//~namespace
