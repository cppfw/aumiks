/**
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once

#include <utki/SpinLock.hpp>
#include <mutex>

#include "Exc.hpp"
#include "Source.hpp"
#include "Frame.hpp"

namespace aumiks{



class Input{
	audout::Frame_e frameType_var;
	
protected:
	std::shared_ptr<aumiks::Source> src;
	
	utki::SpinLock spinLock;
	
	Input(decltype(frameType_var) frameType) :
			frameType_var(frameType)
	{}
public:
	virtual ~Input()noexcept{}
	
	decltype(frameType_var) frameType()const noexcept{
		return this->frameType_var;
	}
	
	void disconnect()noexcept;
	
	void connect(std::shared_ptr<aumiks::Source> source);
	
	//thread safe
	bool isConnected()const{
		return this->src.operator bool();
	}
};



template <audout::Frame_e frame_type> class ChanneledInput : public Input{
	std::shared_ptr<ChanneledSource<frame_type>> srcInUse;

public:
	
	ChanneledInput() :
			Input(frame_type)
	{}
	
	bool fillSampleBuffer(utki::Buf<Frame<frame_type>> buf)noexcept{
		if(this->src != this->srcInUse){
			std::lock_guard<utki::SpinLock> guard(this->spinLock);
			ASSERT(!this->src || this->src->numChannels() == audout::AudioFormat::numChannels(frame_type))
			this->srcInUse = std::dynamic_pointer_cast<typename decltype(srcInUse)::element_type>(this->src);
		}
		if(!this->srcInUse){
			return false;
		}
		return this->srcInUse->fillSampleBuffer(buf);
	}
};


}
