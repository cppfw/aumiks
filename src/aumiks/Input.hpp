/**
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once

#include "Exc.hpp"
#include "Source.hpp"

namespace aumiks{



class Input{
	template <std::uint8_t> friend class ChanneledInput;
	
	std::uint8_t numChannels;
	
	Input(std::uint8_t numChannels) :
			numChannels(numChannels)
	{}
	
	std::shared_ptr<aumiks::Source> src;
	
	utki::SpinLock spinLock;
public:
	virtual ~Input()noexcept{}
	
	std::uint8_t NumChannels()const noexcept{
		return this->numChannels;
	}
	
	void Disconnect()noexcept{
		//To minimize the time with locked spinlock need to avoid object destruction
		//within the locked spinlock period. To achieve that use temporary strong reference.
		std::shared_ptr<aumiks::Source> tmp;
		
		if(this->src){
			std::lock_guard<utki::SpinLock> guard(this->spinLock);
			tmp = this->src;
			tmp->isConnected = false;
			this->src.reset();
		}
	}
	
	void Connect(const std::shared_ptr<aumiks::Source>& source){
		ASSERT(source)
		ASSERT(this->NumChannels() == source->NumChannels())
		
		if(this->IsConnected()){
			throw aumiks::Exc("Input already connected");
		}
		
		if(source->IsConnected()){
			throw aumiks::Exc("Source is already connected");
		}
		
		{
			std::lock_guard<utki::SpinLock> guard(this->spinLock);
			source->isConnected = true;
			this->src = source;
		}
	}
	
	//thread safe
	bool IsConnected()const{
		return this->src.operator bool();
	}
};



template <std::uint8_t num_channels> class ChanneledInput : public Input{
	std::shared_ptr<aumiks::Source> srcInUse;

public:
	
	ChanneledInput() :
			Input(num_channels)
	{}
	
	bool FillSampleBuffer(utki::Buf<std::int32_t> buf)noexcept{
		if(this->src != this->srcInUse){
			std::lock_guard<utki::SpinLock> guard(this->spinLock);
			ASSERT(!this->src || this->src->NumChannels() == num_channels)
			this->srcInUse = this->src;//this->src.template StaticCast<T_ChanneledSource>();
		}
		if(!this->srcInUse){
			return false;
		}
		typedef aumiks::ChanneledSource<num_channels> T_ChanneledOutput;
		return static_cast<T_ChanneledOutput&>(*this->srcInUse).fillSampleBuffer(buf);
	}
};


}
