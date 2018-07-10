#pragma once

#include <utki/Unique.hpp>

#include "Input.hpp"
#include "Exc.hpp"


namespace aumiks{

//TODO: doxygen
template <class T_Sample> class Sink : public utki::Unique{
protected:	
	Sink(){}
public:
	virtual ~Sink()noexcept{}
	
	virtual void start() = 0;
	
	virtual void stop(){
		throw aumiks::Exc("Sink::Stop(): unsupported");
	}
	
	virtual aumiks::Input<T_Sample>& input()noexcept = 0;
};



template <class T_Sample, audout::Frame_e frame_type> class FramedSink : public Sink<T_Sample>{
protected:
	FramedSink(){}
	
protected:
	aumiks::FramedInput<T_Sample, frame_type> input_var;
public:
	constexpr static audout::Frame_e frameType() noexcept{
		return frame_type;
	}
	
	aumiks::Input<T_Sample>& input()noexcept override{
		return this->input_var;
	}
};

}
