#include "../../src/aumiks/Mixer.hpp"
#include "../../src/aumiks/WavSound.hpp"
#include "../../src/aumiks/SpeakersSink.hpp"
#include "../../src/aumiks/NullSource.hpp"

#include <utki/math.hpp>

#include <nitki/Thread.hpp>

#include <cmath>


class SineSource : public aumiks::FramedSource<float, audout::Frame_e::MONO>{
	float t = 0;
public:
	bool fillSampleBuffer(utki::Buf<aumiks::Frame<float, audout::Frame_e::MONO> > buf)noexcept override{
		for(auto d = buf.begin(), e = buf.end(); d != e; ++d){
			d->channel[0] = 0xfff * std::sin(2 * utki::pi<float>() * this->t * 440);
			this->t += 1 / 44100.0f;
		}
		return t > 5;
	}
};



int main(int argc, char *argv[]){
	{
		TRACE_ALWAYS(<< "Opening audio playback device: mono 44100" << std::endl)
		aumiks::MonoSink sink(audout::SamplingRate_e::HZ_44100);
		
		sink.start();
		
		auto mixer = utki::makeShared<aumiks::FramedMixer<std::int32_t, sink.frameType()>>();
		
		std::shared_ptr<aumiks::Sound> snd1 = aumiks::WavSound::load("../samples/sample44100mono16.wav");
		std::shared_ptr<aumiks::Sound> snd2 = aumiks::WavSound::load("../samples/ice_break.wav");

		ASSERT(snd1)
		ASSERT(snd2)

		mixer->connect(snd1->createSource(sink.samplingRate()));
		mixer->connect(snd2->createSource(sink.samplingRate()));
		mixer->connect(utki::makeShared<SineSource>());
		
//		mixer->setFinite(false);
//		mixer->connect(utki::makeShared<aumiks::NullSource<decltype(sink)::sinkFrameType()>>());
		
		sink.input().connect(mixer);
		
		while(sink.input().isConnected()){
			nitki::Thread::sleep(333);
		}
	}
	
	return 0;
}
