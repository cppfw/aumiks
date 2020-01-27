#pragma once

#include <utki/shared.hpp>
#include <utki/span.hpp>

#include <audout/format.hpp>

#include "Frame.hpp"

namespace aumiks{

//TODO: doxygen
class Source : virtual public utki::shared{
	friend class Input;
	
	bool isConnected_v = false;
	
protected:
	Source(const Source&) = delete;
	Source& operator=(const Source&) = delete;
	
	Source(){}
	
	virtual bool fillSampleBuffer(utki::span<Frame> buf)noexcept = 0;
public:	
	virtual ~Source()noexcept{}
	
	//thread safe
	bool isConnected()const noexcept{
		return this->isConnected_v;
	}
private:

};

}
