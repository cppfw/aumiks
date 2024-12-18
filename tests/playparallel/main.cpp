#include "../../src/aumiks/mixer.hpp"
#include "../../src/aumiks/wav_sound.hpp"
#include "../../src/aumiks/speakers.hpp"

#include <utki/math.hpp>

#include <nitki/thread.hpp>

#include <cmath>

class SineSource : public aumiks::source{
	float t = 0;
	
	float limit;
	float freq;
public:
	SineSource(float limit, float freq) :
			limit(limit),
			freq(freq)
	{}
	
	bool fill_sample_buffer(utki::span<aumiks::frame> buf)noexcept override{
		for(auto d = buf.begin(), e = buf.end(); d != e; ++d){
			d->channel[0] = aumiks::real(0xfff * std::sin(2 * utki::pi * this->t * this->freq));
			this->t += 1 / 44100.0f;
		}
		return this->t > this->limit;
	}
};

int main(int argc, char *argv[]){
	{
		utki::log([&](auto&o){o << "Opening audio playback device: mono 44100" << std::endl;});
		aumiks::speakers sink(audout::rate::hz_44100);
		
		sink.start();
		
		auto mixer = std::make_shared<aumiks::mixer>();
		
		auto snd1 = aumiks::wav_sound::load("../samples/sample44100mono16.wav");
		auto snd2 = aumiks::wav_sound::load("../samples/ice_break.wav");

		ASSERT(snd1)
		ASSERT(snd2)

		mixer->connect(snd1->create_source(sink.sampling_rate));
		mixer->connect(snd2->create_source(sink.sampling_rate));
		mixer->connect(std::make_shared<SineSource>(6.0f, 440.0f));
		mixer->connect(std::make_shared<SineSource>(10.0f, 220.0f));
		
//		mixer->setFinite(false);
//		mixer->connect(std::make_shared<aumiks::NullSource<decltype(sink)::sinkFrameType()>>());
		
		sink.input.connect(mixer);
		
		while(sink.input.is_connected()){
			std::this_thread::sleep_for(std::chrono::milliseconds(333));
		}
	}
	
	return 0;
}
