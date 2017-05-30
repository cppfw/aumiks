#pragma once

#include <utki/Shared.hpp>
#include <utki/Buf.hpp>

#include <audout/AudioFormat.hpp>

#include "Frame.hpp"

namespace aumiks{

template <class T_Sample> class Input;

//TODO: doxygen
template<class T_Sample> class Source : virtual public utki::Shared{
	template <class T> friend class Input;
	
	bool isConnected_v = false;
	
protected:
	Source(const Source&) = delete;
	Source& operator=(const Source&) = delete;
	
	Source(){}
public:

	typedef T_Sample Sample_t;
	
	virtual ~Source()noexcept{}
	
	virtual audout::Frame_e frameType()const noexcept = 0;
	
	//thread safe
	bool isConnected()const noexcept{
		return this->isConnected_v;
	}
private:

};

template <class T_Sample> class SingleInputSource : virtual public Source<T_Sample>{
public:
	virtual Input<T_Sample>& input() = 0;
};


template <class T_Sample, audout::Frame_e frame_type> class FramedSource : virtual public Source<T_Sample>{
public:
	FramedSource(const FramedSource&) = delete;
	FramedSource& operator=(const FramedSource&) = delete;
	
	FramedSource(){}
	
	virtual bool fillSampleBuffer(utki::Buf<Frame<T_Sample, frame_type>> buf)noexcept = 0;
	
	audout::Frame_e frameType() const noexcept override{
		return frame_type;
	}
};


}
