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
class MixerBuffer44100Mono16;
class MixerBuffer44100Stereo16;
class PulseAudioBackend;



enum E_Format{
	MONO_16_11025,
	MONO_16_22050,
	MONO_16_44100,
	STEREO_16_11025,
	STEREO_16_22050,
	STEREO_16_44100
};



class Lib : public ting::Singleton<Lib>{
	friend class aumiks::Channel;
	friend class aumiks::PulseAudioBackend;
	
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

public:
	//Base class for mixer buffers of different formats
	class MixerBuffer{
	protected:
		MixerBuffer(unsigned mixBufSize, unsigned playBufSize) :
				mixBuf(mixBufSize),
				smpBuf(mixBufSize),
				playBuf(playBufSize)
		{}

	public:
		virtual ~MixerBuffer(){}

		ting::Array<ting::s32> mixBuf;
		ting::Array<ting::s32> smpBuf;
		ting::Array<ting::u8> playBuf;

		inline void CleanMixBuf(){
			memset(this->mixBuf.Begin(), 0, this->mixBuf.SizeInBytes());
		}
		
		//return true if channel has finished playing and should be removed from playing channels pool
		bool MixToMixBuf(const ting::Ref<aumiks::Channel>& ch);

		void CopyFromMixBufToPlayBuf();
		
	private:
		virtual bool FillSmpBuf(const ting::Ref<aumiks::Channel>& ch) = 0;
		
		void MixSmpBufToMixBuf();
	};

	//base class for audio backends
	class AudioBackend{
	protected:
		inline AudioBackend(){}
	public:
		virtual ~AudioBackend(){}

		virtual void Write(const ting::Buffer<ting::u8>& buf) = 0;
	};
private:
	class SoundThread : public ting::MsgThread{
		const ting::Ptr<AudioBackend> audioBackend;
		const ting::Ptr<MixerBuffer> mixerBuffer;
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
		
	private:
		
		//TODO: create mixer buffer based on actual buffer size and format from the backend
		static ting::Ptr<MixerBuffer> CreateMixerBuffer(unsigned bufferSizeMillis, E_Format format);
	};

	SoundThread thread;
};



//base channel class
class Channel : public ting::RefCounted{
	friend class Lib;
	friend class Lib::SoundThread;
	friend class aumiks::MixerBuffer44100Mono16;
	friend class aumiks::MixerBuffer44100Stereo16;

	ting::Inited<volatile bool, false> isPlaying;
	
protected:
	ting::Inited<unsigned, 0> numLoops;//0 means loop infinitely

protected:
	ting::Inited<bool, false> stopFlag;//TODO: should it be volatile?

	ting::Inited<ting::u8, ting::u8(-1)> volume;
	
	Channel(){}
	
public:

	inline bool IsPlaying()const{
		return this->isPlaying;
	}

	inline void Play(unsigned numLoops = 1){
		this->numLoops = numLoops;//TODO: should it be under mutex protection?
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
	virtual bool FillSmpBuf11025Mono16(ting::Buffer<ting::s32>& mixBuf){return true;}
	virtual bool FillSmpBuf11025Stereo16(ting::Buffer<ting::s32>& mixBuf){return true;}
	virtual bool FillSmpBuf22050Mono16(ting::Buffer<ting::s32>& mixBuf){return true;}
	virtual bool FillSmpBuf22050Stereo16(ting::Buffer<ting::s32>& mixBuf){return true;}
	virtual bool FillSmpBuf44100Mono16(ting::Buffer<ting::s32>& mixBuf){return true;}
	virtual bool FillSmpBuf44100Stereo16(ting::Buffer<ting::s32>& mixBuf){return true;}
};



//base class for all sounds
class Sound : public ting::RefCounted{
protected:
	Sound(){}
public:


};

} //~namespace
