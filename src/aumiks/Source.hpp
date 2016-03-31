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
	
	virtual audout::AudioFormat::EFrame frameType()const noexcept = 0;
	
	unsigned numChannels()const noexcept{
		return audout::AudioFormat::numChannels(this->frameType());
	}
	
	//thread safe
	bool isConnected()const noexcept{
		return this->isConnected_var;
	}
private:

};


template <audout::AudioFormat::EFrame frame_type> class ChanneledSource : virtual public Source{
public:
	ChanneledSource(const ChanneledSource&) = delete;
	ChanneledSource& operator=(const ChanneledSource&) = delete;
	
	ChanneledSource(){}
	
	virtual bool fillSampleBuffer(utki::Buf<Frame<frame_type>> buf)noexcept = 0;
	
	audout::AudioFormat::EFrame frameType() const noexcept override{
		return frame_type;
	}
};


}
