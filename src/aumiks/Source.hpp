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
	
	audout::AudioFormat::EFrame frameType_var;
	
protected:
	Source(const Source&) = delete;
	Source& operator=(const Source&) = delete;
	
	Source(decltype(frameType_var) frameType) :
			frameType_var(frameType)
	{}
public:
	
	virtual ~Source()noexcept{}
	
	decltype(frameType_var) frameType()const noexcept{
		return this->frameType_var;
	}
	
	
	unsigned numChannels()const noexcept{
		return audout::AudioFormat::numChannels(this->frameType_var);
	}
	
	//thread safe
	bool isConnected()const noexcept{
		return this->isConnected_var;
	}
private:

};


template <audout::AudioFormat::EFrame frame_type> class ChanneledSource : public Source{
public:
	ChanneledSource(const ChanneledSource&) = delete;
	ChanneledSource& operator=(const ChanneledSource&) = delete;
	
	ChanneledSource() :
			Source(frame_type)
	{}
	
	virtual bool fillSampleBuffer(utki::Buf<Frame<frame_type>> buf)noexcept = 0;
};


}
