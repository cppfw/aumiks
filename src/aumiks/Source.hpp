/**
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once

#include <utki/Shared.hpp>
#include <utki/Buf.hpp>

#include <audout/AudioFormat.hpp>

#include "Frame.hpp"

namespace aumiks{


//TODO: doxygen
class ASource : virtual public utki::Shared{
	friend class Input;
	
	bool isConnected_var = false;
	
protected:
	ASource(const ASource&) = delete;
	ASource& operator=(const ASource&) = delete;
	
	ASource(){}
public:
	
	virtual ~ASource()noexcept{}
	
	virtual audout::Frame_e frameType()const noexcept = 0;
	
	//thread safe
	bool isConnected()const noexcept{
		return this->isConnected_var;
	}
private:

};


template <audout::Frame_e frame_type> class Source : virtual public ASource{
public:
	Source(const Source&) = delete;
	Source& operator=(const Source&) = delete;
	
	Source(){}
	
	virtual bool fillSampleBuffer(utki::Buf<Frame<frame_type>> buf)noexcept = 0;
	
	audout::Frame_e frameType() const noexcept override{
		return frame_type;
	}
};


}
