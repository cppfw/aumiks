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

protected:	
	Sink(decltype(frameType_var) frameType) :
			frameType_var(frameType)
	{}
public:
	
	std::uint8_t numChannels()const noexcept{
		return audout::AudioFormat::numChannels(this->frameType_var);
	}
	
	virtual ~Sink()noexcept{}
	
	virtual void start() = 0;
	
	virtual void stop(){
		throw aumiks::Exc("Sink::Stop(): unsupported");
	}
	
	virtual aumiks::Input& input()noexcept = 0;
};



template <audout::AudioFormat::EFrame frame_type> class ChanneledSink : public Sink{
protected:
	ChanneledSink() :
			Sink(frame_type)
	{}
	
protected:
	aumiks::ChanneledInput<frame_type> input_var;
public:
	aumiks::Input& input()noexcept override{
		return this->input_var;
	}

};

}
