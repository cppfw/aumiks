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
	audout::AudioFormat::EFrame frameType_var;
	std::uint32_t frequency;

protected:	
	Sink(decltype(frameType_var) frameType, std::uint32_t frequency) :
			frameType_var(frameType),
			frequency(frequency)
	{}
public:
	
	std::uint8_t NumChannels()const noexcept{
		return audout::AudioFormat::numChannels(this->frameType_var);
	}
	
	std::uint32_t Frequency()const noexcept{
		return this->frequency;
	}
	
	virtual ~Sink()noexcept{}
	
	virtual void Start() = 0;
	
	virtual void Stop(){
		throw aumiks::Exc("Sink::Stop(): unsupported");
	}
	
	virtual aumiks::Input& input()noexcept = 0;
};



template <audout::AudioFormat::EFrame frame_type> class ChanneledSink : public Sink{
protected:
	ChanneledSink(std::uint32_t frequency) :
			Sink(frame_type, frequency)
	{}
	
protected:
	aumiks::ChanneledInput<frame_type> input_var;
public:
	aumiks::Input& input()noexcept override{
		return this->input_var;
	}

};

}
