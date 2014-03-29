/*
 * File:   SpeakersSink.cpp
 * Author: igagis
 *
 * Created on March 21, 2014, 5:48 PM
 */

#include "SpeakersSink.hpp"

using namespace aumiks;

SpeakersSink::SpeakersSink(audout::AudioFormat outputFormat, ting::u16 bufferSizeMillis) :
		smpBuf((outputFormat.samplingRate.Frequency() * bufferSizeMillis / 1000) * outputFormat.frame.NumChannels())
{
	this->player = audout::Player::CreatePlayer(outputFormat, smpBuf.Size() / outputFormat.frame.NumChannels(), this);
}

//override
void SpeakersSink::Start(){
	this->player->SetPaused(false);
}

//override
void SpeakersSink::Stop(){
	this->player->SetPaused(true);
}
