#include "../../src/aumiks/WavSound.hpp"
#include "../../src/aumiks/SpeakersSink.hpp"

#include <papki/FSFile.hpp>
#include <nitki/Thread.hpp>


//void Play(const std::string& fileName){
//	TRACE_ALWAYS(<< "Playing " << fileName << std::endl)
//	std::shared_ptr<aumiks::WavSound> snd = aumiks::WavSound::Load(fileName);
//
//	ASSERT(snd)
//	
//	std::shared_ptr<aumiks::Source> ch = snd->CreateSource();
//	ch->Play_ts();
//	
//	while(!ch->IsStopped_ts()){
////		TRACE(<< "Loop" << std::endl)
//		ting::mt::Thread::Sleep(333);
//	}
//}


int main(int argc, char *argv[]){
	{
		aumiks::SpeakersSink sink(audout::SamplingRate_e::HZ_22050);
		
		sink.start();
		
		std::shared_ptr<aumiks::Sound> snd = aumiks::WavSound::load("../samples/sample44100stereo16.wav");
		
		auto src = snd->createSource(sink.samplingRate());
		
		//test disconnect and connect again
//		sink.input.connect(src);
//		sink.input.disconnect();
		sink.input.connect(src);
		
		while(sink.input.isConnected()){
			nitki::Thread::sleep(333);
		}
	}
	
	{
		aumiks::SpeakersSink sink(audout::SamplingRate_e::HZ_22050);
		
		sink.start();
		
		std::shared_ptr<aumiks::Sound> snd = aumiks::WavSound::load("../samples/sample11025stereo16.wav");
		
		sink.input.connect(snd->createSource(sink.samplingRate()));
		
		
		while(sink.input.isConnected()){
			nitki::Thread::sleep(333);
		}
	}
	
	
//	{
//		TRACE_ALWAYS(<< "Opening audio playback device: Mono 11025" << std::endl)
//		aumiks::Lib aumiksLibrary(audout::AudioFormat(audout::AudioFormat::Frame::MONO, audout::AudioFormat::SamplingRate::HZ_11025), 100);
//
//		Play("../samples/sample11025mono16.wav");
//		Play("../samples/sample11025stereo16.wav");
//		Play("../samples/sample22050mono16.wav");
//		Play("../samples/sample22050stereo16.wav");
//		Play("../samples/sample44100mono16.wav");
//		Play("../samples/sample44100stereo16.wav");
//	}
//	
//	{
//		TRACE_ALWAYS(<< "Opening audio playback device: Stereo 11025" << std::endl)
//		aumiks::Lib aumiksLibrary(audout::AudioFormat(audout::AudioFormat::Frame::STEREO, audout::AudioFormat::SamplingRate::HZ_11025), 100);
//
//		Play("../samples/sample11025mono16.wav");
//		Play("../samples/sample11025stereo16.wav");
//		Play("../samples/sample22050mono16.wav");
//		Play("../samples/sample22050stereo16.wav");
//		Play("../samples/sample44100mono16.wav");
//		Play("../samples/sample44100stereo16.wav");
//	}
//	
//	{
//		TRACE_ALWAYS(<< "Opening audio playback device: Mono 22050" << std::endl)
//		aumiks::Lib aumiksLibrary(audout::AudioFormat(audout::AudioFormat::Frame::MONO, audout::AudioFormat::SamplingRate::HZ_22050), 100);
//
//		Play("../samples/sample11025mono16.wav");
//		Play("../samples/sample11025stereo16.wav");
//		Play("../samples/sample22050mono16.wav");
//		Play("../samples/sample22050stereo16.wav");
//		Play("../samples/sample44100mono16.wav");
//		Play("../samples/sample44100stereo16.wav");
//	}
//	
//	{
//		TRACE_ALWAYS(<< "Opening audio playback device: Stereo 22050" << std::endl)
//		aumiks::Lib aumiksLibrary(audout::AudioFormat(audout::AudioFormat::Frame::STEREO, audout::AudioFormat::SamplingRate::HZ_22050), 100);
//
//		Play("../samples/sample11025mono16.wav");
//		Play("../samples/sample11025stereo16.wav");
//		Play("../samples/sample22050mono16.wav");
//		Play("../samples/sample22050stereo16.wav");
//		Play("../samples/sample44100mono16.wav");
//		Play("../samples/sample44100stereo16.wav");
//	}
//	
//	{
//		TRACE_ALWAYS(<< "Opening audio playback device: Mono 44100" << std::endl)
//		aumiks::Lib aumiksLibrary(audout::AudioFormat(audout::AudioFormat::Frame::MONO, audout::AudioFormat::SamplingRate::HZ_44100), 100);
//
//		Play("../samples/sample11025mono16.wav");
//		Play("../samples/sample11025stereo16.wav");
//		Play("../samples/sample22050mono16.wav");
//		Play("../samples/sample22050stereo16.wav");
//		Play("../samples/sample44100mono16.wav");
//		Play("../samples/sample44100stereo16.wav");
//	}
//	
//	{
//		TRACE_ALWAYS(<< "Opening audio playback device: Stereo 44100" << std::endl)
//		aumiks::Lib aumiksLibrary(audout::AudioFormat(audout::AudioFormat::Frame::STEREO, audout::AudioFormat::SamplingRate::HZ_44100), 100);
//
//		Play("../samples/sample11025mono16.wav");
//		Play("../samples/sample11025stereo16.wav");
//		Play("../samples/sample22050mono16.wav");
//		Play("../samples/sample22050stereo16.wav");
//		Play("../samples/sample44100mono16.wav");
//		Play("../samples/sample44100stereo16.wav");
//	}
	
	return 0;
}
