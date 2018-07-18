#pragma once

#include <utki/SpinLock.hpp>

#include <type_traits>
#include <mutex>

#include "config.hpp"
#include "Exc.hpp"
#include "Source.hpp"
#include "Frame.hpp"

namespace aumiks{

class Input{
protected:
	std::shared_ptr<Source> src;
	
	utki::SpinLock mutex;
	
	Input(){}
public:
	virtual ~Input()noexcept{}
	
	virtual audout::Frame_e frameType()const noexcept = 0;
	
	void disconnect()noexcept{
		std::lock_guard<decltype(this->mutex)> guard(this->mutex);
		
		if (this->src) {
			this->src->isConnected_v = false;
			this->src.reset();
		}
	}
	
	void connect(std::shared_ptr<Source> source);
	
	bool isConnected()const{
		return this->src.get() != nullptr;
	}
	
};


template <audout::Frame_e frame_type> class FramedInput : public Input{
	std::shared_ptr<FramedSource<frame_type>> srcInUse;

public:

	FramedInput(){}

	audout::Frame_e frameType() const noexcept override{
		return frame_type;
	}

	bool fillSampleBuffer(utki::Buf<Frame<frame_type>> buf)noexcept{
		{
			std::lock_guard<decltype(this->mutex)> guard(this->mutex);
			if(this->src != std::static_pointer_cast<aumiks::Source>(this->srcInUse)){
				ASSERT(!this->src || this->src->frameType() == frame_type)
				this->srcInUse = std::dynamic_pointer_cast<typename decltype(srcInUse)::element_type>(this->src);
			}
		}
		if(!this->srcInUse){
			for(auto& b : buf){
				for(auto& c : b.channel){
					c = 0;
				}
			}
			return false;
		}
		return this->srcInUse->fillSampleBuffer(buf);
	}
};

}
