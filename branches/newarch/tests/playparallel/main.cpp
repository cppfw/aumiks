#include "../../src/aumiks/Lib.hpp"
#include "../../src/aumiks/WavSound.hpp"



int main(int argc, char *argv[]){
	TRACE_ALWAYS(<< "Opening audio playback device: mono 44100" << std::endl)
	aumiks::Lib aumiksLibrary(44100, 1, 100);
	
	ting::Ref<aumiks::WavSound> snd1 = aumiks::WavSound::LoadWAV("../samples/sample44100mono16.wav");
	ting::Ref<aumiks::WavSound> snd2 = aumiks::WavSound::LoadWAV("../samples/ice_break.wav");

	ASSERT(snd1)
	ASSERT(snd2)
	
	TRACE_ALWAYS(<< "Playing 1" << std::endl)
	ting::Ref<aumiks::Channel> ch1 = snd1->CreateChannel();
	ch1->Play_ts();
	
	ting::mt::Thread::Sleep(1000);
	
	TRACE_ALWAYS(<< "Playing 2" << std::endl)
	ting::Ref<aumiks::Channel> ch2 = snd2->CreateChannel();
	ch2->Play_ts();
	
	while(!ch1->IsStopped_ts() || !ch2->IsStopped_ts()){
//		TRACE(<< "Loop" << std::endl)
		ting::mt::Thread::Sleep(50);
	}
	
	return 0;
}
