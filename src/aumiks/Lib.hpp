/**
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once


#include <utki/Singleton.hpp>

#include "MixChannel.hpp"

#include <audout/AudioFormat.hpp>
#include <audout/Listener.hpp>
#include <audout/Player.hpp>

#include "Mixer.hpp"


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
class Lib : public ting::IntrusiveSingleton<Lib>, private audout::PlayerListener{
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
	
	typedef std::list<std::unique_ptr<Action> > T_ActionsList;
	typedef T_ActionsList::iterator T_ActionsIter;
	
	T_ActionsList actionsList1, actionsList2;
	T_ActionsList *addList, *handleList;
	
	audout::AudioFormat outputFormat;

	std::shared_ptr<utki::Shared> mixer;
	
	template <std::uint8_t num_channels> aumiks::Mixer<std::int32_t, num_channels>& MasterMixer()throw(){
		return *static_cast<aumiks::Mixer<std::int32_t, num_channels>*>(this->mixer.operator->());
	}
	
	ting::Array<std::int32_t> smpBuf;
	
	std::unique_ptr<audout::Player> player;

public:
	inline const audout::AudioFormat& OutputFormat()throw(){
		return this->outputFormat;
	}
	
	
	
	//TODO: re-doxygen
	/**
	 * @brief Create sound library singleton instance.
	 * Creates singleton instance of sound library object and
	 * opens sound device.
	 * @param outputFormat - desired audio output format.
	 * @param bufferSizeMillis - size of desired playing buffer in milliseconds. Use smaller buffers for higher latency.
	 *                           Note, that very small buffer may result in bigger overhead and lags. The same applies to very big buffer sizes.
	 */
	Lib(audout::AudioFormat outputFormat, std::uint16_t bufferSizeMillis = 100);
	
	
	
	~Lib()throw();
	
	
	
	template <class T_Sample, std::uint8_t num_channels> void PlaySource(std::shared_ptr<ChanSource<T_Sample, num_channels> >& s){
		//TODO:
	}
	
	

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
	void CopySmpBufToPlayBuf(utki::Buf<std::int16_t>& playBuf);
	
	//this function is not thread-safe, but it is supposed to be called from special audio thread
	//override
	void FillPlayBuf(utki::Buf<std::int16_t>& playBuf);
	
	
	inline void PushAction_ts(std::unique_ptr<Action> action){
		ting::atomic::SpinLock::Guard mutexGuard(this->actionsSpinLock);
		this->addList->push_back(action);
	}
		
public:
	//TODO:
//	/**
//	 * @brief Add global effect.
//	 * Adds the effect to the list of global effects which are applied to the
//	 * final mixing buffer after all the playing channels are mixed.
//	 * @param effect - effect to add.
//	 */
//	inline void AddEffect_ts(const std::shared_ptr<Effect>& effect){
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
//	inline void RemoveEffect_ts(const std::shared_ptr<Effect>& effect){
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
