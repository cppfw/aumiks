/*
 * File:   SpeakersSink.cpp
 * Author: igagis
 *
 * Created on March 21, 2014, 5:48 PM
 */

#include "SpeakersSink.hpp"

using namespace aumiks;

SpeakersSink::SpeakersSink(audout::AudioFormat::SamplingRate::Type samplingRate, ting::u16 bufferSizeMillis) :
		smpBuf((audout::AudioFormat::SamplingRate(samplingRate).Frequency() * bufferSizeMillis / 1000) * this->NumChannels())
{
	this->player = audout::Player::CreatePlayer(
			audout::AudioFormat(frame_type, samplingRate),
			smpBuf.Size() / this->NumChannels(),
			this
		);
}

//override
void SpeakersSink::Start(){
	this->player->SetPaused(false);
}

//override
void SpeakersSink::Stop(){
	this->player->SetPaused(true);
}
