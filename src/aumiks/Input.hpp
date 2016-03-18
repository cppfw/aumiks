/**
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once

#include <utki/SpinLock.hpp>
#include <mutex>

#include "Exc.hpp"
#include "Source.hpp"

namespace aumiks{



class Input{
	audout::AudioFormat::EFrame frameType_var;
	std::uint32_t frequency_var;
	
protected:
	std::shared_ptr<aumiks::Source> src;
	
	utki::SpinLock spinLock;
	
	Input(decltype(frameType_var) frameType, decltype(frequency_var) frequency) :
			frameType_var(frameType),
			frequency_var(frequency)
	{}
public:
	virtual ~Input()noexcept{}
	
	decltype(frameType_var) frameType()const noexcept{
		return this->frameType_var;
	}
	
	decltype(frequency_var) frequency()const noexcept{
		return this->frequency_var;
	}
	
	void Disconnect()noexcept;
	
	void Connect(std::shared_ptr<aumiks::Source> source);
	
	//thread safe
	bool IsConnected()const{
		return this->src.operator bool();
	}
};



template <audout::AudioFormat::EFrame frame_type> class ChanneledInput : public Input{
	std::shared_ptr<aumiks::Source> srcInUse;

public:
	
	ChanneledInput(std::uint32_t frequency) :
			Input(frame_type, frequency)
	{}
	
	bool FillSampleBuffer(utki::Buf<std::int32_t> buf)noexcept{
		if(this->src != this->srcInUse){
			std::lock_guard<utki::SpinLock> guard(this->spinLock);
			ASSERT(!this->src || this->src->NumChannels() == audout::AudioFormat::numChannels(frame_type))
			this->srcInUse = this->src;//this->src.template StaticCast<T_ChanneledSource>();
		}
		if(!this->srcInUse){
			return false;
		}
		typedef aumiks::ChanneledSource<frame_type> T_ChanneledOutput;
		return static_cast<T_ChanneledOutput&>(*this->srcInUse).fillSampleBuffer(buf);
	}
};


}
