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

class SingleInputSource : virtual public Source{
public:
	virtual Input& input() = 0;
};


template <audout::Frame_e frame_type> class FramedSource : virtual public Source{
public:
	FramedSource(const FramedSource&) = delete;
	FramedSource& operator=(const FramedSource&) = delete;
	
	FramedSource(){}
	
	virtual bool fillSampleBuffer(utki::Buf<Frame<frame_type>> buf)noexcept = 0;
	
	audout::Frame_e frameType() const noexcept override{
		return frame_type;
	}
};


}
