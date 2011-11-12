#include "../../src/aumiks/WavSound.hpp"

#include <ting/FSFile.hpp>


void Play(const std::string& fileName){
	TRACE_ALWAYS(<< "Playing " << fileName << std::endl)
	ting::Ref<aumiks::WavSound> snd = aumiks::WavSound::LoadWAV(fileName);

	ASSERT(snd)
	
	ting::Ref<aumiks::Channel> ch = snd->Play();
	
	while(ch->IsPlaying()){
//		TRACE(<< "Loop" << std::endl)
		ting::Thread::Sleep(50);
	}
}


int main(int argc, char *argv[]){
	aumiks::Lib aumiksLibrary(100, aumiks::STEREO_16_44100);
	
	Play("sample11025mono16.wav");
	Play("sample11025stereo16.wav");
	Play("sample22050mono16.wav");
	Play("sample22050stereo16.wav");
	Play("sample44100mono16.wav");
	Play("sample44100stereo16.wav");
	
	return 0;
}
