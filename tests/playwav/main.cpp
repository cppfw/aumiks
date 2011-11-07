#include "../../src/aumiks/WavSound.hpp"

#include <ting/FSFile.hpp>



int main(int argc, char *argv[]){
	aumiks::Lib aumiksLibrary(100, aumiks::STEREO_16_44100);
	
	ting::Ref<aumiks::WavSound> snd;
	{
		ting::FSFile fi("sample.wav");
		snd = aumiks::WavSound::LoadWAV(fi);
	}

	ting::Ref<aumiks::Channel> ch = snd->Play();
	
	while(ch->IsPlaying()){
//		TRACE(<< "Loop" << std::endl)
		ting::Thread::Sleep(50);
	}

	return 0;
}
