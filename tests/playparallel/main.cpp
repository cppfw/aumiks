#include "../../src/aumiks/Mixer.hpp"
#include "../../src/aumiks/WavSound.hpp"
#include "../../src/aumiks/SpeakersSink.hpp"
#include "../../src/aumiks/NullSource.hpp"



int main(int argc, char *argv[]){
	{
		TRACE_ALWAYS(<< "Opening audio playback device: mono 44100" << std::endl)
		aumiks::MonoSink sink(audout::SamplingRate_e::HZ_44100);
		
		sink.start();
		
		auto mixer = utki::makeShared<aumiks::Mixer<sink.frameType()>>();
		
		std::shared_ptr<aumiks::Sound> snd1 = aumiks::WavSound::load("../samples/sample44100mono16.wav");
		std::shared_ptr<aumiks::Sound> snd2 = aumiks::WavSound::load("../samples/ice_break.wav");

		ASSERT(snd1)
		ASSERT(snd2)

		mixer->connect(snd1->createSource(sink.samplingRate()));
		mixer->connect(snd2->createSource(sink.samplingRate()));
		
//		mixer->setFinite(false);
//		mixer->connect(utki::makeShared<aumiks::NullSource<decltype(sink)::sinkFrameType()>>());
		
		sink.input().connect(mixer);
		
		while(sink.input().isConnected()){
			nitki::Thread::sleep(333);
		}
	}
	
	return 0;
}
