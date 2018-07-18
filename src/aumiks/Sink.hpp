#pragma once

#include <utki/Unique.hpp>

#include "Input.hpp"
#include "Exc.hpp"


namespace aumiks{

//TODO: doxygen
class Sink : public utki::Unique{
protected:	
	Sink(){}
public:
	virtual ~Sink()noexcept{}
	
	virtual void start() = 0;
	
	virtual void stop(){
		throw Exc("Sink::Stop(): unsupported");
	}
	
	virtual Input& input()noexcept = 0;
};



template <audout::Frame_e frame_type> class FramedSink : public Sink{
protected:
	FramedSink(){}
	
protected:
	aumiks::FramedInput<frame_type> input_v;
public:
	constexpr static audout::Frame_e frameType() noexcept{
		return frame_type;
	}
	
	aumiks::Input& input()noexcept override{
		return this->input_v;
	}
};

}
