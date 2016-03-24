#include "../../src/aumiks/Lib.hpp"
#include "../../src/aumiks/WavSound.hpp"



int main(int argc, char *argv[]){
	TRACE_ALWAYS(<< "Opening audio playback device: mono 44100" << std::endl)
	aumiks::Lib aumiksLibrary(audout::AudioFormat(audout::AudioFormat::Frame::MONO, audout::AudioFormat::SamplingRate::HZ_44100), 100);
	
	std::shared_ptr<aumiks::WavSound> snd1 = aumiks::WavSound::load("../samples/sample44100mono16.wav");
	std::shared_ptr<aumiks::WavSound> snd2 = aumiks::WavSound::load("../samples/ice_break.wav");

	ASSERT(snd1)
	ASSERT(snd2)
	
	TRACE_ALWAYS(<< "Playing 1" << std::endl)
	std::shared_ptr<aumiks::Channel> ch1 = snd1->CreateChannel();
	ch1->Play_ts();
	
	ting::mt::Thread::Sleep(1000);
	
	TRACE_ALWAYS(<< "Playing 2" << std::endl)
	std::shared_ptr<aumiks::Channel> ch2 = snd2->CreateChannel();
	ch2->Play_ts();
	
	while(!ch1->IsStopped_ts() || !ch2->IsStopped_ts()){
//		TRACE(<< "Loop" << std::endl)
		ting::mt::Thread::Sleep(50);
	}
	
	return 0;
}
