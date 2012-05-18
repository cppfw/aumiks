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


#include <ting/debug.hpp>
#include <ting/Singleton.hpp>
#include <ting/atomic.hpp>

#include "MixChannel.hpp"



//#define M_ENABLE_AUMIKS_TRACE
#ifdef M_ENABLE_AUMIKS_TRACE
#define M_AUMIKS_TRACE(x) TRACE(<< "[aumiks] ") TRACE(x)
#else
#define M_AUMIKS_TRACE(x)
#endif



namespace aumiks{



/**
 * @brief aumiks library singleton class.
 * This is a main class of the aumiks library.
 * Before using the library one has to create a single instance of the Lib class.
 * It will perform necessary sound output initializations and open sound output device.
 * Destroying the object will close the sound output device and clean all the resources.
 */
class Lib : public ting::IntrusiveSingleton<Lib>{
	friend class ting::IntrusiveSingleton<Lib>;
	static ting::IntrusiveSingleton<Lib>::T_Instance instance;

	friend class aumiks::Channel;
	friend class aumiks::MixChannel;
	
	
	class Action{
	public:
		virtual ~Action()throw(){}
		
		virtual void Perform() = 0;
	};
	
	
	ting::atomic::SpinLock actionsSpinLock;
	
	typedef std::list<ting::Ptr<Action> > T_ActionsList;
	typedef T_ActionsList::iterator T_ActionsIter;
	
	T_ActionsList actionsList1, actionsList2;
	T_ActionsList *addList, *handleList;
	
	
	
	//sampling rate
	unsigned freq;
	
	//channels (mono, stereo, quadro, 5.1, 7.1, etc.)
	unsigned chans;
	
	//size of the playing buffer in frames
	unsigned bufSizeInFrames;
	
	inline unsigned BufSizeInSamples()const throw(){
		return this->bufSizeInFrames * this->chans;
	}

	ting::Ref<aumiks::MixChannel> masterChannel;
	
	ting::Array<ting::s32> smpBuf;
	
	void *backend;

public:
	
	
	//TODO: doxygen comments
	inline const ting::Ref<aumiks::MixChannel>& MasterChannel()throw(){
		return this->masterChannel;
	}
	
	
	
	inline void PlayChannel_ts(const ting::Ref<aumiks::Channel>& channel){
		this->MasterChannel()->PlayChannel_ts(channel);
	}
	
	
	
	/**
	 * @brief Create sound library singleton instance.
	 * Creates singleton instance of sound library object and
	 * opens sound device.
	 * @param bufferSizeMillis - size of desired playing buffer in milliseconds. Use smaller buffers for higher latency.
	 *                           Note, that very small buffer may result in bigger overhead and lags. The same applies to very big buffer sizes.
	 * @param freq - sampling rate in Hertz.
	 * @param chans - number of channels. 1 = mono, 2 = stereo, etc.
	 */
	Lib(unsigned freq, unsigned chans, ting::u16 bufferSizeMillis = 100);
	
	
	
	~Lib()throw();
	
	

	/**
	 * @brief Mute sound output.
	 * Mute the sound output. This is the same as the resulting sound volume would be set to zero.
	 * I.e. all the computational resources for sound mixing etc. are still being consumed when sound is muted.
	 * @param muted - pass true to mute the sound, false to un-mute.
	 */
	//TODO:
//	inline void SetMuted(bool muted){
//		this->mixerBuffer->isMuted = muted;
//	}
//
//	/**
//	 * @brief Unmute the sound.
//	 * Inversion of Lib::SetMuted() method.
//	 * See description of Lib::SetMuted() method for more info.
//	 * @param unmuted - true to un-mute the sound, false to mute.
//	 */
//	inline void SetUnmuted(bool unmuted){
//		this->SetMuted(!unmuted);
//	}
//
//	/**
//	 * @brief Mute the sound.
//	 * See description of Lib::SetMuted() method for more info.
//	 */
//	inline void Mute(){
//		this->SetMuted(true);
//	}
//
//	/**
//	 * @brief Un-mute the sound.
//	 * See description of Lib::SetMuted() method for more info.
//	 */
//	inline void Unmute(){
//		this->SetMuted(false);
//	}
//
//	/**
//	 * @brief Check if sound output is muted or not.
//	 * @return true if sound output is muted.
//	 * @return false otherwise.
//	 */
//	inline bool IsMuted()const{
//		return this->mixerBuffer->isMuted;
//	}
//
//	/**
//	 * @brief Sets the paused state of the audio engine.
//	 * Moves engine to paused or resumed state depending on the passed parameter value.
//	 * In paused state the engine still holds the audio device open but
//	 * does not play the main audio buffer, thus does not consume CPU resources.
//	 * The method is not thread-safe and should be called from the thread where Lib object was created.
//	 * @param pause - determines whether to pause or resume the audio engine. Pass true to pause and false to resume.
//	 */
//	inline void SetPaused(bool pause){
//		ASSERT(this->audioBackend)
//		this->audioBackend->SetPaused(pause);
//	}
//
//	/**
//	 * @brief Pause audio engine.
//	 * Moves the audio engine to paused state.
//	 * Essentially it just calls the SetPaused_ts(true) method.
//	 * The method is not thread-safe and should be called from the thread where Lib object was created.
//	 */
//	inline void Pause(){
//		this->SetPaused(true);
//	}
//	
//	/**
//	 * @brief Resume audio engine.
//	 * Un-pauses the audio engine. See Pause_ts() method for more info.
//	 * Essentially it just calls the SetPaused_ts(false) method.
//	 * The method is not thread-safe and should be called from the thread where Lib object was created.
//	 */
//	inline void Resume(){
//		this->SetPaused(false);
//	}


private:
	void CopySmpBufToPlayBuf(ting::Buffer<ting::u8>& playBuf);
	
	//this function is not thread-safe, but it is supposed to be called from special audio thread
	void FillPlayBuf(ting::Buffer<ting::u8>& playBuf);
	
	//TODO: remove?
//	void PlayChannel_ts(const ting::Ref<Channel>& ch);
	
public:
	//TODO:
//	/**
//	 * @brief Add global effect.
//	 * Adds the effect to the list of global effects which are applied to the
//	 * final mixing buffer after all the playing channels are mixed.
//	 * @param effect - effect to add.
//	 */
//	inline void AddEffect_ts(const ting::Ref<Effect>& effect){
//		ASSERT(effect.IsValid())
//		
//		//TODO:
//	}
//
//	/**
//	 * @brief Remove global effect.
//	 * Removes the effect from the list of global effects which are applied to the
//	 * final mixing buffer after all the playing channels are mixed.
//	 * @param effect - effect to remove.
//	 */
//	inline void RemoveEffect_ts(const ting::Ref<Effect>& effect){
//		ASSERT(effect.IsValid())
//
//		//TODO:
//	}
//
//	inline void RemoveAllEffects_ts(){
//
//		//TODO:
//	}
};


}//~namespace
