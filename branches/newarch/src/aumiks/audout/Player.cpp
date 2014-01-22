/* 
 * File:   Player.cpp
 * Author: igagis
 * 
 * Created on January 22, 2014, 3:52 PM
 */

#include <ting/config.hpp>

#include "Player.hpp"

#if M_OS == M_OS_WINDOWS
#	include "backend/DirectSoundBackend.hpp"
#elif M_OS == M_OS_LINUX
#	if M_OS_NAME == M_OS_NAME_ANDROID
#		include "backend/OpenSLESBackend.hpp"
#	else
#		include "backend/PulseAudioBackend.hpp"
//#		include "backend/ALSABackend.hpp"
#	endif
#else
#	error "Unknown OS"
#endif

using namespace audout;

Player::Player(PlayerListener* listener) :
		listener(listener)
{
}

Player::~Player()throw(){
}


//static
ting::Ptr<Player> Player::CreatePlayer(AudioFormat outputFormat, ting::u32 bufSizeInFrames, PlayerListener* listener){
	Player* ret;
#if M_OS == M_OS_WINDOWS
	ret = new DirectSoundBackend(this->bufSizeInFrames, freq, chans);
#elif M_OS == M_OS_LINUX
#	if M_OS_NAME == M_OS_NAME_ANDROID
	ret = new OpenSLESBackend(this->bufSizeInFrames, freq, chans);
#	else
	ret = new PulseAudioBackend(outputFormat, bufSizeInFrames, listener);
//	ret = new ALSABackend(this->bufSizeInFrames, freq, chans);
#	endif
#else
#	error "undefined OS"
#endif
	return ting::Ptr<Player>(ret);
}
