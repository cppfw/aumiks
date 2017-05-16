#pragma once


#include <papki/File.hpp>

#include "Sound.hpp"



namespace aumiks{



class WavSound : public aumiks::Sound{
	
	std::uint8_t chans;
	std::uint32_t freq;
	
protected:
	WavSound(std::uint8_t chans, std::int32_t freq) :
			chans(chans),
			freq(freq)
	{}

public:
	std::uint8_t numChannels()const throw(){
		return this->chans;
	}
	
	std::uint32_t samplingRate()const throw(){
		return this->freq;
	}
	
	decltype(freq) frequency()const noexcept{
		return this->freq;
	}

	static std::shared_ptr<WavSound> load(const std::string& fileName);
	static std::shared_ptr<WavSound> load(papki::File& fi);
};



}
