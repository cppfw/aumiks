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


//TODO: remove this file


#pragma once

#include <ting/types.hpp>
#include <ting/Singleton.hpp>
#include <ting/Exc.hpp>
#include <ting/types.hpp>
#include <ting/debug.hpp>
#include <ting/Buffer.hpp>
#include <ting/Array.hpp>
#include <ting/Ref.hpp>
#include <ting/Thread.hpp>
#include <ting/Ptr.hpp>

#include <list>

#include "Exc.hpp"



//#define M_ENABLE_AUMIKS_TRACE
#ifdef M_ENABLE_AUMIKS_TRACE
#define M_AUMIKS_TRACE(x) TRACE(<< "[aumiks] ") TRACE(x)
#else
#define M_AUMIKS_TRACE(x)
#endif



namespace aumiks{



//forward declarations
class Lib;
class Channel;
class MixChannel;
class AudioBackend;











/**
 * @brief Returns frame size for given sound output format.
 * Returns number of bytes per frame for given sound output format.
 * The sound frame is the sound data for a single sampling rate tick.
 * Each frame consists of number of channels samples (e.g. mono: 1 frame = 1 sample, stereo: 1 frame = 2 samples).
 * @param format - format to return the frame size of.
 * @return the size of the frame in bytes.
 */
unsigned BytesPerFrame(E_Format format);



/**
 * @brief Returns number of samples per frame for given sound output format.
 * In fact, this function returns number of channels of the given sound output format.
 * @param format - format for which to return the frame size in samples.
 * @return size of the frame in samples.
 */
unsigned SamplesPerFrame(E_Format format);



/**
 * @brief Returns sampling rate for given sound output format.
 * @return Sampling rate in Hz.
 */
unsigned SamplingRate(E_Format format);















//Full template specializations
template <> inline bool Lib::MixerBuffer::FillSmpBufImpl<11025, 1>(const ting::Ref<aumiks::Channel>& ch){
	return ch->FillSmpBuf11025Mono16(this->smpBuf);
}
template <> inline bool Lib::MixerBuffer::FillSmpBufImpl<11025, 2>(const ting::Ref<aumiks::Channel>& ch){
	return ch->FillSmpBuf11025Stereo16(this->smpBuf);
}
template <> inline bool Lib::MixerBuffer::FillSmpBufImpl<22050, 1>(const ting::Ref<aumiks::Channel>& ch){
	return ch->FillSmpBuf22050Mono16(this->smpBuf);
}
template <> inline bool Lib::MixerBuffer::FillSmpBufImpl<22050, 2>(const ting::Ref<aumiks::Channel>& ch){
	return ch->FillSmpBuf22050Stereo16(this->smpBuf);
}
template <> inline bool Lib::MixerBuffer::FillSmpBufImpl<44100, 1>(const ting::Ref<aumiks::Channel>& ch){
	return ch->FillSmpBuf44100Mono16(this->smpBuf);
}
template <> inline bool Lib::MixerBuffer::FillSmpBufImpl<44100, 2>(const ting::Ref<aumiks::Channel>& ch){
	return ch->FillSmpBuf44100Stereo16(this->smpBuf);
}



//Full template specializations
template <> inline Effect::E_Result Effect::ApplyToBufImpl<11025, 1>(ting::Buffer<ting::s32>& buf, bool soundStopped){
	return this->ApplyToBuf11025Mono16(buf, soundStopped);
}
template <> inline Effect::E_Result Effect::ApplyToBufImpl<11025, 2>(ting::Buffer<ting::s32>& buf, bool soundStopped){
	return this->ApplyToBuf11025Stereo16(buf, soundStopped);
}
template <> inline Effect::E_Result Effect::ApplyToBufImpl<22050, 1>(ting::Buffer<ting::s32>& buf, bool soundStopped){
	return this->ApplyToBuf22050Mono16(buf, soundStopped);
}
template <> inline Effect::E_Result Effect::ApplyToBufImpl<22050, 2>(ting::Buffer<ting::s32>& buf, bool soundStopped){
	return this->ApplyToBuf22050Stereo16(buf, soundStopped);
}
template <> inline Effect::E_Result Effect::ApplyToBufImpl<44100, 1>(ting::Buffer<ting::s32>& buf, bool soundStopped){
	return this->ApplyToBuf44100Mono16(buf, soundStopped);
}
template <> inline Effect::E_Result Effect::ApplyToBufImpl<44100, 2>(ting::Buffer<ting::s32>& buf, bool soundStopped){
	return this->ApplyToBuf44100Stereo16(buf, soundStopped);
}











} //~namespace
