/**
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once

#include <utki/Unique.hpp>

#include "Input.hpp"
#include "Exc.hpp"


namespace aumiks{

//TODO: doxygen
class Sink : utki::Unique{
	template <std::uint8_t> friend class ChanneledSink;
	
	std::uint8_t numChannels;
	std::uint32_t frequency;
	
	Sink(std::uint8_t numChannles, std::uint32_t frequency) :
			numChannels(numChannles),
			frequency(frequency)
	{}
public:
	
	std::uint8_t NumChannels()const noexcept{
		return this->numChannels;
	}
	
	std::uint32_t Frequency()const noexcept{
		return this->frequency;
	}
	
	virtual ~Sink()noexcept{}
	
	virtual void Start() = 0;
	
	virtual void Stop(){
		throw aumiks::Exc("Sink::Stop(): unsupported");
	}
};



template <std::uint8_t num_channels> class ChanneledSink : public Sink{
protected:
	ChanneledSink(std::uint32_t frequency) :
			Sink(num_channels, frequency)
	{}
public:
	aumiks::ChanneledInput<num_channels> input;
};

}
