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
class Source : virtual public utki::Shared{
	friend class Input;
	
	bool isConnected_var = false;
	
protected:
	Source(const Source&) = delete;
	Source& operator=(const Source&) = delete;
	
	Source(){}
public:
	
	virtual ~Source()noexcept{}
	
	virtual audout::Frame_e frameType()const noexcept = 0;
	
	//thread safe
	bool isConnected()const noexcept{
		return this->isConnected_var;
	}
private:

};


template <audout::Frame_e frame_type> class ChanneledSource : virtual public Source{
public:
	ChanneledSource(const ChanneledSource&) = delete;
	ChanneledSource& operator=(const ChanneledSource&) = delete;
	
	ChanneledSource(){}
	
	virtual bool fillSampleBuffer(utki::Buf<Frame<frame_type>> buf)noexcept = 0;
	
	audout::Frame_e frameType() const noexcept override{
		return frame_type;
	}
};


}
