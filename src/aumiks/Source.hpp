#pragma once

#include <utki/Shared.hpp>
#include <utki/Buf.hpp>

#include <audout/AudioFormat.hpp>

#include "Frame.hpp"

namespace aumiks{

class Input;

//TODO: doxygen
class Source : virtual public utki::Shared{
	friend class Input;
	
	bool isConnected_v = false;
	
protected:
	Source(const Source&) = delete;
	Source& operator=(const Source&) = delete;
	
	Source(){}
	
	virtual bool fillSampleBuffer(utki::Buf<Frame> buf)noexcept = 0;
public:	
	virtual ~Source()noexcept{}
	
	//thread safe
	bool isConnected()const noexcept{
		return this->isConnected_v;
	}
private:

};

}
