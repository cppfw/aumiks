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
protected:
	std::shared_ptr<aumiks::Source> src;
	
	utki::SpinLock spinLock;
	
	Input(){}
public:
	virtual ~Input()noexcept{}
	
	virtual audout::Frame_e frameType()const noexcept = 0;
	
	void disconnect()noexcept;
	
	void connect(std::shared_ptr<aumiks::Source> source);
	
	bool isConnected()const{
		return this->src.operator bool();
	}
};



template <audout::Frame_e frame_type> class ChanneledInput : public Input{
	std::shared_ptr<ChanneledSource<frame_type>> srcInUse;

public:

	ChanneledInput(){}

	audout::Frame_e frameType() const noexcept override{
		return frame_type;
	}

	bool fillSampleBuffer(utki::Buf<Frame<frame_type>> buf)noexcept{
		if(this->src != this->srcInUse){
			std::lock_guard<utki::SpinLock> guard(this->spinLock);
			ASSERT(!this->src || this->src->frameType() == frame_type)
			this->srcInUse = std::dynamic_pointer_cast<typename decltype(srcInUse)::element_type>(this->src);
		}
		if(!this->srcInUse){
			return false;
		}
		return this->srcInUse->fillSampleBuffer(buf);
	}
};


}
