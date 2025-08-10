#include <cmath>

#include <nitki/thread.hpp>
#include <utki/math.hpp>

#include "../../src/aumiks/mixer.hpp"
#include "../../src/aumiks/speakers.hpp"
#include "../../src/aumiks/wav_sound.hpp"

class sine_source : public aumiks::source
{
	float t = 0;

	float duration;
	float freq;

public:
	sine_source(
		float duration, //
		float freq
	) :
		duration(duration),
		freq(freq)
	{}

	bool fill_sample_buffer(utki::span<aumiks::frame> buf) noexcept override
	{
		for (auto& d : buf) {
			constexpr auto aplitude = 0xfff;
			constexpr auto sampling_rate = 44100;
			d.channel[0] = aumiks::real(aplitude * std::sin(2 * utki::pi * this->t * this->freq));
			this->t += 1 / float(sampling_rate);
		}
		return this->t > this->duration;
	}
};

int main(int argc, char* argv[])
{
	{
		utki::log([&](auto& o) {
			o << "Opening audio playback device: mono 44100" << std::endl;
		});
		aumiks::speakers sink(audout::rate::hz_44100);

		sink.start();

		auto mixer = std::make_shared<aumiks::mixer>();

		auto snd1 = aumiks::wav_sound::load("../samples/sample44100mono16.wav");
		auto snd2 = aumiks::wav_sound::load("../samples/ice_break.wav");

		ASSERT(snd1)
		ASSERT(snd2)

		mixer->connect(snd1->create_source(sink.sampling_rate));
		mixer->connect(snd2->create_source(sink.sampling_rate));
		mixer->connect(std::make_shared<sine_source>(
			6.0f, // NOLINT
			440.0f // NOLINT
		));
		mixer->connect(std::make_shared<sine_source>(
			10.0f, // NOLINT
			220.0f // NOLINT
		));

		//		mixer->setFinite(false);
		//		mixer->connect(std::make_shared<aumiks::NullSource<decltype(sink)::sinkFrameType()>>());

		sink.input.connect(mixer);

		while (sink.input.is_connected()) {
			constexpr auto delay_ms = 333;
			std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
		}
	}

	return 0;
}
