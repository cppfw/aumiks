#pragma once


#include <papki/File.hpp>

#include "Sound.hpp"

namespace aumiks{

class WavSound : public aumiks::Sound{
public:
	const std::uint8_t numChannels;
	const std::uint32_t samplingRate;
	
protected:
	WavSound(std::uint8_t chans, std::int32_t freq) :
			numChannels(chans),
			samplingRate(freq)
	{}

public:
	static std::shared_ptr<WavSound> load(const std::string& fileName);
	static std::shared_ptr<WavSound> load(papki::File& fi);
};

}
