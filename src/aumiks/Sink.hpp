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
	
	Input input;
};

}
