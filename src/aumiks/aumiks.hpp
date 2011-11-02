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



enum E_Format{
	MONO_8_11025,
	MONO_16_11025,
	STEREO_8_11025,
	STEREO_16_11025,
	MONO_8_22050,
	MONO_16_22050,
	STEREO_8_22050,
	STEREO_16_22050,
	MONO_8_44100,
	MONO_16_44100,
	STEREO_8_44100,
	STEREO_16_44100,
	MONO_8_48000,
	MONO_16_48000,
	STEREO_8_48000,
	STEREO_16_48000
};



class Lib : public ting::Singleton<Lib>{
	friend class Channel;

public:
	/**
	 * @brief Create sound library singleton instance.
	 * Creates singleton instance of sound library object and
	 * opens sound device.
	 * @param bufferSizeMillis - size of desired playing buffer in milliseconds.
	 * @param format - desired format of sound output.
	 */
	Lib(unsigned bufferSizeMillis = 100, aumiks::E_Format format = STEREO_16_22050);
	
	~Lib();

	void PlayChannel(ting::Ref<Channel> ch);

	inline void SetMuted(bool muted){
		this->thread.isMuted = muted;
	}

	inline void SetUnmuted(bool unmuted){
		this->SetMuted(!unmuted);
	}

	inline void Mute(){
		this->SetMuted(true);
	}

	inline void Unmute(){
		this->SetMuted(false);
	}

	inline bool IsMuted()const{
		return this->thread.isMuted;
	}

private:
	class SoundThread : public ting::MsgThread{
		E_Format format;
	public:
		ting::Mutex chPoolMutex;
		
		typedef std::list<ting::Ref<aumiks::Channel> > TChPool;
		typedef TChPool::iterator TChIter;
		TChPool chPool;

		TChPool chPoolToAdd;

		ting::Inited<volatile bool, false> isMuted;

		SoundThread(unsigned bufferSizeMillis, E_Format format);

		//override
		void Run();
	};

	SoundThread thread;
};



//base channel class
class Channel : public ting::RefCounted{
	friend class Lib;
	friend class Lib::SoundThread;

	ting::Inited<volatile bool, false> isPlaying;
	
protected:
	ting::Inited<bool, false> stopFlag;

	ting::Inited<ting::u8, ting::u8(-1)> volume;
	
	Channel(){}
	
public:

	inline bool IsPlaying()const{
		return this->isPlaying;
	}

	inline void Play(){
		aumiks::Lib::Inst().PlayChannel(ting::Ref<aumiks::Channel>(this));
	}

	inline void Stop(){
		this->stopFlag = true;
	}

	inline void SetVolume(ting::u8 vol){
		this->volume = vol;
	}

protected:
	/**
	 * @brief Called when channel has been added to pool of playing channels.
	 */
	virtual void OnStart(){}
private:
	//this function is called by SoundThread when it needs more data to play.
	//return true to remove channel from playing channels list
	virtual bool MixToMixBuf(ting::Array<ting::s32>& mixBuf) = 0;
};



//base class for all sounds
class Sound : public ting::RefCounted{
protected:
	Sound(){}
public:


};

} //~namespace
