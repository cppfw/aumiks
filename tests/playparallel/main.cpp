#include "../../src/aumiks/Mixer.hpp"
#include "../../src/aumiks/WavSound.hpp"
#include "../../src/aumiks/Speakers.hpp"
#include "../../src/aumiks/NullSource.hpp"

#include <utki/math.hpp>

#include <nitki/thread.hpp>

#include <cmath>


class SineSource : public aumiks::Source{
	float t = 0;
	
	float limit;
	float freq;
public:
	SineSource(float limit, float freq) :
			limit(limit),
			freq(freq)
	{}
	
	bool fillSampleBuffer(utki::span<aumiks::Frame> buf)noexcept override{
		for(auto d = buf.begin(), e = buf.end(); d != e; ++d){
			d->channel[0] = 0xfff * std::sin(2 * utki::pi<float>() * this->t * this->freq);
			this->t += 1 / 44100.0f;
		}
		return this->t > this->limit;
	}
};

int main(int argc, char *argv[]){
	{
		TRACE_ALWAYS(<< "Opening audio playback device: mono 44100" << std::endl)
		aumiks::Speakers sink(audout::rate::hz44100);
		
		sink.start();
		
		auto mixer = std::make_shared<aumiks::Mixer>();
		
		std::shared_ptr<aumiks::Sound> snd1 = aumiks::WavSound::load("../samples/sample44100mono16.wav");
		std::shared_ptr<aumiks::Sound> snd2 = aumiks::WavSound::load("../samples/ice_break.wav");

		ASSERT(snd1)
		ASSERT(snd2)

		mixer->connect(snd1->createSource(sink.samplingRate));
		mixer->connect(snd2->createSource(sink.samplingRate));
		mixer->connect(std::make_shared<SineSource>(6.0f, 440.0f));
		mixer->connect(std::make_shared<SineSource>(10.0f, 220.0f));
		
//		mixer->setFinite(false);
//		mixer->connect(std::make_shared<aumiks::NullSource<decltype(sink)::sinkFrameType()>>());
		
		sink.input.connect(mixer);
		
		while(sink.input.isConnected()){
			std::this_thread::sleep_for(std::chrono::milliseconds(333));
		}
	}
	
	return 0;
}
