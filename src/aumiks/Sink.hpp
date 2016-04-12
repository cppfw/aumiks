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
protected:	
	Sink(){}
public:
	virtual ~Sink()noexcept{}
	
	virtual void start() = 0;
	
	virtual void stop(){
		throw aumiks::Exc("Sink::Stop(): unsupported");
	}
	
	virtual aumiks::Input& input()noexcept = 0;
};



template <audout::Frame_e frame_type> class ChanneledSink : public Sink{
protected:
	ChanneledSink(){}
	
protected:
	aumiks::ChanneledInput<frame_type> input_var;
public:
	constexpr static audout::Frame_e frameType() noexcept{
		return frame_type;
	}
	
	aumiks::Input& input()noexcept override{
		return this->input_var;
	}
};

}
