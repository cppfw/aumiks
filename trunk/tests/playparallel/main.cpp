#include "../../src/aumiks/WavSound.hpp"



int main(int argc, char *argv[]){
	TRACE_ALWAYS(<< "Opening audio playback device: Stereo 44100" << std::endl)
	aumiks::Lib aumiksLibrary(100, aumiks::STEREO_16_44100);
	
	ting::Ref<aumiks::WavSound> snd1 = aumiks::WavSound::LoadWAV("../samples/sample11025mono16.wav");
	ting::Ref<aumiks::WavSound> snd2 = aumiks::WavSound::LoadWAV("../samples/ice_break.wav");

	ASSERT(snd1)
	ASSERT(snd2)
	
	TRACE_ALWAYS(<< "Playing 1" << std::endl)
	ting::Ref<aumiks::Channel> ch1 = snd1->Play();
	
	ting::Thread::Sleep(500);
	
	TRACE_ALWAYS(<< "Playing 2" << std::endl)
	ting::Ref<aumiks::Channel> ch2 = snd2->Play();
	
	while(ch1->IsPlaying() || ch2->IsPlaying()){
//		TRACE(<< "Loop" << std::endl)
		ting::Thread::Sleep(50);
	}
	
	return 0;
}
