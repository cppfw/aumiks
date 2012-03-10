#include "../../src/aumiks/WavSound.hpp"

#include <ting/fs/FSFile.hpp>


void Play(const std::string& fileName){
	TRACE_ALWAYS(<< "Playing " << fileName << std::endl)
	ting::Ref<aumiks::WavSound> snd = aumiks::WavSound::LoadWAV(fileName);

	ASSERT(snd)
	
	ting::Ref<aumiks::WavSound::Channel> ch = snd->CreateWavChannel();
	ch->Play(2);
	
	while(ch->IsPlaying()){
//		TRACE(<< "Loop" << std::endl)
		ting::Thread::Sleep(50);
	}
}


int main(int argc, char *argv[]){
	{
		TRACE_ALWAYS(<< "Opening audio playback device: Mono 11025" << std::endl)
		aumiks::Lib aumiksLibrary(100, aumiks::MONO_16_11025);

		Play("../samples/sample11025mono16.wav");
		Play("../samples/sample11025stereo16.wav");
		Play("../samples/sample22050mono16.wav");
		Play("../samples/sample22050stereo16.wav");
		Play("../samples/sample44100mono16.wav");
		Play("../samples/sample44100stereo16.wav");
	}
	
	{
		TRACE_ALWAYS(<< "Opening audio playback device: Stereo 11025" << std::endl)
		aumiks::Lib aumiksLibrary(100, aumiks::STEREO_16_11025);

		Play("../samples/sample11025mono16.wav");
		Play("../samples/sample11025stereo16.wav");
		Play("../samples/sample22050mono16.wav");
		Play("../samples/sample22050stereo16.wav");
		Play("../samples/sample44100mono16.wav");
		Play("../samples/sample44100stereo16.wav");
	}
	
	{
		TRACE_ALWAYS(<< "Opening audio playback device: Mono 22050" << std::endl)
		aumiks::Lib aumiksLibrary(100, aumiks::MONO_16_22050);

		Play("../samples/sample11025mono16.wav");
		Play("../samples/sample11025stereo16.wav");
		Play("../samples/sample22050mono16.wav");
		Play("../samples/sample22050stereo16.wav");
		Play("../samples/sample44100mono16.wav");
		Play("../samples/sample44100stereo16.wav");
	}
	
	{
		TRACE_ALWAYS(<< "Opening audio playback device: Stereo 22050" << std::endl)
		aumiks::Lib aumiksLibrary(100, aumiks::STEREO_16_22050);

		Play("../samples/sample11025mono16.wav");
		Play("../samples/sample11025stereo16.wav");
		Play("../samples/sample22050mono16.wav");
		Play("../samples/sample22050stereo16.wav");
		Play("../samples/sample44100mono16.wav");
		Play("../samples/sample44100stereo16.wav");
	}
	
	{
		TRACE_ALWAYS(<< "Opening audio playback device: Mono 44100" << std::endl)
		aumiks::Lib aumiksLibrary(100, aumiks::MONO_16_44100);

		Play("../samples/sample11025mono16.wav");
		Play("../samples/sample11025stereo16.wav");
		Play("../samples/sample22050mono16.wav");
		Play("../samples/sample22050stereo16.wav");
		Play("../samples/sample44100mono16.wav");
		Play("../samples/sample44100stereo16.wav");
	}
	
	{
		TRACE_ALWAYS(<< "Opening audio playback device: Stereo 44100" << std::endl)
		aumiks::Lib aumiksLibrary(100, aumiks::STEREO_16_44100);

		Play("../samples/sample11025mono16.wav");
		Play("../samples/sample11025stereo16.wav");
		Play("../samples/sample22050mono16.wav");
		Play("../samples/sample22050stereo16.wav");
		Play("../samples/sample44100mono16.wav");
		Play("../samples/sample44100stereo16.wav");
	}
	
	return 0;
}
