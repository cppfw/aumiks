/* The MIT License:

Copyright (c) 2014 Ivan Gagis

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

#include <audout/Player.hpp>

#include "Sink.hpp"

namespace aumiks{

template <audout::AudioFormat::Frame::Type frame_type> class SpeakersSink :
		public aumiks::ChanSink<audout::AudioFormat::Frame::Traits<frame_type>::NUM_CHANNELS>,
		private audout::PlayerListener
{
	SpeakersSink(const SpeakersSink&);

	ting::Ptr<audout::Player> player;

	ting::Array<ting::s32> smpBuf;

	//this function is not thread-safe, but it is supposed to be called from special audio thread
	//override
	void FillPlayBuf(const ting::Buffer<ting::s16>& playBuf)throw(){
		ASSERT(this->smpBuf.Size() == playBuf.Size())

		if(this->input.FillSampleBuffer(this->smpBuf)){
			this->input.Disconnect();
		}
		
		const ting::s32 *src = this->smpBuf.Begin();
		ting::s16* dst = playBuf.Begin();
		for(; src != this->smpBuf.End(); ++src, ++dst){
			ting::s32 tmp = *src;
			ting::util::ClampTop(tmp, 0x7fff);
			ting::util::ClampBottom(tmp, -0x7fff);

			ASSERT(playBuf.Overlaps(dst))
			
			*dst = ting::s16(tmp);
		}
		ASSERT(dst == playBuf.End())
	}
	
public:
	SpeakersSink(audout::AudioFormat::SamplingRate::Type samplingRate, ting::u16 bufferSizeMillis = 100) :
			aumiks::ChanSink<audout::AudioFormat::Frame::Traits<frame_type>::NUM_CHANNELS>(
					audout::AudioFormat::SamplingRate(samplingRate).Frequency()
				),
			smpBuf((this->Frequency() * bufferSizeMillis / 1000) * this->NumChannels())
	{
		this->player = audout::Player::CreatePlayer(
				audout::AudioFormat(frame_type, samplingRate),
				smpBuf.Size() / this->NumChannels(),
				this
			);
	}
	
	//override
	void Start(){
		this->player->SetPaused(false);
	}

	//override
	void Stop(){
		this->player->SetPaused(true);
	}
};



typedef SpeakersSink<audout::AudioFormat::Frame::MONO> MonoSink;
typedef SpeakersSink<audout::AudioFormat::Frame::STEREO> StereoSink;



}
