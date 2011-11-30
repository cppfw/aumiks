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



class Effect : public ting::RefCounted{
public:
	/**
	 * @brief Called every time when the Channel is about to start playing.
	 * Called from separate thread.
     */
	virtual void Init_ts(){}
	
	/**
	 * @brief Called when effect is removed from channel.
	 * This method is called when this effect is about to be removed from channel.
	 * It is also called when the channel holding this effect is being destroyed.
     */
	virtual void OnRemoveFromChannel_ts(){}
	
	/**
	 * @brief Called when effect is to be applied to a portion of a playing sound.
     * @param buf - buffer containing portion of sound data.
     * @return true if effect has finished. TODO: explain more
	 * @return false otherwise.
     */
	//TODO: what if effect has suppressed sound (fade out) and further sound playing is pointless
	//      what if sound has stopped, but effect has a delay and should stop later
	virtual bool ApplyToSmpBuf11025Mono16(ting::Buffer<ting::s32>& buf){return true;}
	
	//TODO: add doxygen docs for each method
	virtual bool ApplyToSmpBuf11025Stereo16(ting::Buffer<ting::s32>& buf){return true;}
	virtual bool ApplyToSmpBuf22050Mono16(ting::Buffer<ting::s32>& buf){return true;}
	virtual bool ApplyToSmpBuf22050Stereo16(ting::Buffer<ting::s32>& buf){return true;}
	virtual bool ApplyToSmpBuf44100Mono16(ting::Buffer<ting::s32>& buf){return true;}
	virtual bool ApplyToSmpBuf44100Stereo16(ting::Buffer<ting::s32>& buf){return true;}
};



//base channel class
class Channel : public ting::RefCounted{
	friend class aumiks::Lib;

	ting::Inited<volatile bool, false> isPlaying;
	
	bool soundStopped;//used to indicate that sound has finished playing, but effects are still playing.

	ting::Inited<volatile bool, false> stopFlag;//indicates that playing should stop immediately
	
	//list of effects
	typedef std::list<ting::Ref<aumiks::Effect> > T_EffectsList;
	typedef T_EffectsList::iterator T_EffectsIter;
	T_EffectsList effects;
	
	inline void IniteEffects(){
		for(T_EffectsIter i = this->effects.begin(); i != this->effects.end(); ++i){
			(*i)->Init_ts();
		}
	}
	
	inline void RemoveEffect(const ting::Ref<Effect>& e){
		for(T_EffectsIter i = this->effects.begin(); i != this->effects.end(); ++i){
			if((*i) == e){
				this->effects.erase(i);
				break;
			}
		}
	}
	
	inline bool ApplyEffectsToSmpBuf11025Mono16(ting::Buffer<ting::s32>& buf){
		bool ret = true;
		//return true if all effects returned true;
		for(T_EffectsIter i = this->effects.begin(); i != this->effects.end(); ++i){
			ret &= (*i)->ApplyToSmpBuf11025Mono16(buf);
		}
		return ret;
	}
	inline bool ApplyEffectsToSmpBuf11025Stereo16(ting::Buffer<ting::s32>& buf){
		bool ret = true;
		//return true if all effects returned true;
		for(T_EffectsIter i = this->effects.begin(); i != this->effects.end(); ++i){
			ret &= (*i)->ApplyToSmpBuf11025Stereo16(buf);
		}
		return ret;
	}
	inline bool ApplyEffectsToSmpBuf22050Mono16(ting::Buffer<ting::s32>& buf){
		bool ret = true;
		//return true if all effects returned true;
		for(T_EffectsIter i = this->effects.begin(); i != this->effects.end(); ++i){
			ret &= (*i)->ApplyToSmpBuf22050Mono16(buf);
		}
		return ret;
	}
	inline bool ApplyEffectsToSmpBuf22050Stereo16(ting::Buffer<ting::s32>& buf){
		bool ret = true;
		//return true if all effects returned true;
		for(T_EffectsIter i = this->effects.begin(); i != this->effects.end(); ++i){
			ret &= (*i)->ApplyToSmpBuf22050Stereo16(buf);
		}
		return ret;
	}
	inline bool ApplyEffectsToSmpBuf44100Mono16(ting::Buffer<ting::s32>& buf){
		bool ret = true;
		//return true if all effects returned true;
		for(T_EffectsIter i = this->effects.begin(); i != this->effects.end(); ++i){
			ret &= (*i)->ApplyToSmpBuf44100Mono16(buf);
		}
		return ret;
	}
	inline bool ApplyEffectsToSmpBuf44100Stereo16(ting::Buffer<ting::s32>& buf){
		bool ret = true;
		//return true if all effects returned true;
		for(T_EffectsIter i = this->effects.begin(); i != this->effects.end(); ++i){
			ret &= (*i)->ApplyToSmpBuf44100Stereo16(buf);
		}
		return ret;
	}
	
protected:
	ting::Inited<volatile ting::s8, 0> panning;
	
	Channel(){}
	
public:
	~Channel();

	inline bool IsPlaying()const{
		return this->isPlaying;
	}

	inline void Play();

	inline void Stop(){
		this->stopFlag = true;
	}
	
	inline void AddEffect_ts(const ting::Ref<aumiks::Effect>& effect);
	
	inline void RemoveEffect_ts(const ting::Ref<aumiks::Effect>& effect);
	
protected:
	/**
	 * @brief Called when channel has been added to pool of playing channels.
	 */
	virtual void OnStart(){}
	
	/**
	 * @brief Called when channel has been removed from pool of playing channels.
	 */
	virtual void OnStop(){}
private:
	//this function is called by SoundThread when it needs more data to play.
	//return true to remove channel from playing channels list
	virtual bool FillSmpBuf11025Mono16(ting::Buffer<ting::s32>& buf){return true;}
	virtual bool FillSmpBuf11025Stereo16(ting::Buffer<ting::s32>& buf){return true;}
	virtual bool FillSmpBuf22050Mono16(ting::Buffer<ting::s32>& buf){return true;}
	virtual bool FillSmpBuf22050Stereo16(ting::Buffer<ting::s32>& buf){return true;}
	virtual bool FillSmpBuf44100Mono16(ting::Buffer<ting::s32>& buf){return true;}
	virtual bool FillSmpBuf44100Stereo16(ting::Buffer<ting::s32>& buf){return true;}
};



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
	
	void AddEffectToChannel_ts(const ting::Ref<Channel>& ch, const ting::Ref<aumiks::Effect>& eff);
	
	void RemoveEffectFromChannel_ts(const ting::Ref<Channel>& ch, const ting::Ref<aumiks::Effect>& eff);
	
	void PlayChannel_ts(const ting::Ref<Channel>& ch);
	
	static inline void RemoveEffectFromChannelSync(const ting::Ref<Channel>& ch, const ting::Ref<aumiks::Effect>& eff){
		ch->RemoveEffect(eff);
	}
	
	static inline void AddEffectToChannelSync(const ting::Ref<Channel>& ch, const ting::Ref<aumiks::Effect>& eff){
		ch->effects.push_back(eff);
	}
public:
	/**
	 * @brief Create sound library singleton instance.
	 * Creates singleton instance of sound library object and
	 * opens sound device.
	 * @param bufferSizeMillis - size of desired playing buffer in milliseconds.
	 * @param format - format of sound output.
	 */
	Lib(ting::u16 bufferSizeMillis = 100, aumiks::E_Format format = STEREO_16_22050);
	
	~Lib();
	
	inline void SetMuted(bool muted){
		this->thread.mixerBuffer->isMuted = muted;
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
		return this->thread.mixerBuffer->isMuted;
	}

public:
	//Base class for mixer buffers of different formats
	class MixerBuffer{
		friend class aumiks::Lib;
	protected:
		MixerBuffer(unsigned mixBufSize) :
				mixBuf(mixBufSize),
				smpBuf(mixBufSize),
				playBuf(mixBufSize * 2) //2 bytes per sample
		{}

		inline bool FillSmpBuf11025Mono16(const ting::Ref<aumiks::Channel>& ch){
			return ch->FillSmpBuf11025Mono16(this->smpBuf);
		}
		inline bool FillSmpBuf11025Stereo16(const ting::Ref<aumiks::Channel>& ch){
			return ch->FillSmpBuf11025Stereo16(this->smpBuf);
		}
		inline bool FillSmpBuf22050Mono16(const ting::Ref<aumiks::Channel>& ch){
			return ch->FillSmpBuf22050Mono16(this->smpBuf);
		}
		inline bool FillSmpBuf22050Stereo16(const ting::Ref<aumiks::Channel>& ch){
			return ch->FillSmpBuf22050Stereo16(this->smpBuf);
		}
		inline bool FillSmpBuf44100Mono16(const ting::Ref<aumiks::Channel>& ch){
			return ch->FillSmpBuf44100Mono16(this->smpBuf);
		}
		inline bool FillSmpBuf44100Stereo16(const ting::Ref<aumiks::Channel>& ch){
			return ch->FillSmpBuf44100Stereo16(this->smpBuf);
		}
		
		inline bool ApplyEffectsToSmpBuf11025Mono16(const ting::Ref<aumiks::Channel>& ch){
			return ch->ApplyEffectsToSmpBuf11025Mono16(this->smpBuf);
		}
		inline bool ApplyEffectsToSmpBuf11025Stereo16(const ting::Ref<aumiks::Channel>& ch){
			return ch->ApplyEffectsToSmpBuf11025Stereo16(this->smpBuf);
		}
		inline bool ApplyEffectsToSmpBuf22050Mono16(const ting::Ref<aumiks::Channel>& ch){
			return ch->ApplyEffectsToSmpBuf22050Mono16(this->smpBuf);
		}
		inline bool ApplyEffectsToSmpBuf22050Stereo16(const ting::Ref<aumiks::Channel>& ch){
			return ch->ApplyEffectsToSmpBuf22050Stereo16(this->smpBuf);
		}
		inline bool ApplyEffectsToSmpBuf44100Mono16(const ting::Ref<aumiks::Channel>& ch){
			return ch->ApplyEffectsToSmpBuf44100Mono16(this->smpBuf);
		}
		inline bool ApplyEffectsToSmpBuf44100Stereo16(const ting::Ref<aumiks::Channel>& ch){
			return ch->ApplyEffectsToSmpBuf44100Stereo16(this->smpBuf);
		}
		
		ting::Array<ting::s32> mixBuf;
		ting::Array<ting::s32> smpBuf;
		ting::Array<ting::u8> playBuf;

		ting::Inited<volatile bool, false> isMuted;
	public:
		virtual ~MixerBuffer(){}
		
	private:
		inline void CleanMixBuf(){
			memset(this->mixBuf.Begin(), 0, this->mixBuf.SizeInBytes());
		}
		
		//return true if channel has finished playing and should be removed from playing channels pool
		bool MixToMixBuf(const ting::Ref<aumiks::Channel>& ch);

		void CopyFromMixBufToPlayBuf();
		
		virtual bool FillSmpBuf(const ting::Ref<aumiks::Channel>& ch) = 0;
		
		virtual bool ApplyEffectsToSmpBuf(const ting::Ref<aumiks::Channel>& ch) = 0;
		
		void MixSmpBufToMixBuf();
	};

	//base class for audio backends
	class AudioBackend{
		friend class aumiks::Lib;
		
		virtual void Write(const ting::Buffer<ting::u8>& buf) = 0;
	protected:
		inline AudioBackend(){}
	public:
		virtual ~AudioBackend(){}
	};
private:
	class SoundThread : public ting::MsgThread{
	public:
		const ting::Ptr<AudioBackend> audioBackend;
		const ting::Ptr<MixerBuffer> mixerBuffer;
		
		ting::Mutex chPoolMutex;
		
		typedef std::list<ting::Ref<aumiks::Channel> > T_ChPool;
		typedef T_ChPool::iterator T_ChIter;
		T_ChPool chPool;

		T_ChPool chPoolToAdd;
		
		typedef std::pair<ting::Ref<aumiks::Channel>, ting::Ref<aumiks::Effect> > T_ChannelEffectPair;
		typedef std::list<T_ChannelEffectPair> T_ChannelEffectPairsList;
		typedef T_ChannelEffectPairsList::iterator T_ChannelEffectPairsIter;
		T_ChannelEffectPairsList effectsToAdd;
		T_ChannelEffectPairsList effectsToRemove;

		SoundThread(ting::u16 bufferSizeMillis, E_Format format);

		//override
		void Run();
		
	private:
		static ting::Ptr<MixerBuffer> CreateMixerBuffer(unsigned bufferSizeMillis, E_Format format);
	};

	SoundThread thread;
};



//base class for all sounds
class Sound : public ting::RefCounted{
protected:
	Sound(){}
public:
	virtual ting::Ref<aumiks::Channel> CreateChannel()const = 0;
};



inline void Channel::Play(){
	aumiks::Lib::Inst().PlayChannel_ts(ting::Ref<aumiks::Channel>(this));
}



inline void Channel::AddEffect_ts(const ting::Ref<aumiks::Effect>& effect){
	aumiks::Lib::Inst().AddEffectToChannel_ts(
			ting::Ref<Channel>(this),
			effect
		);
}



inline void Channel::RemoveEffect_ts(const ting::Ref<aumiks::Effect>& effect){
	aumiks::Lib::Inst().RemoveEffectFromChannel_ts(
			ting::Ref<Channel>(this),
			effect
		);
}



} //~namespace
