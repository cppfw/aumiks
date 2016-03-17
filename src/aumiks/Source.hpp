/**
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once

#include <utki/Shared.hpp>


namespace aumiks{


//TODO: doxygen
class Source : virtual public utki::Shared{
	friend class Input;
	
	bool isConnected = false;
	
	std::uint8_t numChannels;
	
protected:
	Source(const Source&) = delete;
	Source& operator=(const Source&) = delete;
	
	Source(decltype(numChannels) numChannels) :
			numChannels(numChannels)
	{}
public:
	
	virtual ~Source()noexcept{}
	
	decltype(numChannels) NumChannels()const noexcept{
		return this->numChannels;
	}
	
	//thread safe
	bool IsConnected()const noexcept{
		return this->isConnected;
	}
private:

};


template <std::uint8_t num_channels> class ChanneledSource : public Source{
public:
	ChanneledSource(const ChanneledSource&) = delete;
	ChanneledSource& operator=(const ChanneledSource&) = delete;
	
	ChanneledSource() :
			Source(num_channels)
	{}
	
	virtual bool fillSampleBuffer(utki::Buf<std::int32_t> buf)noexcept = 0;
};


}
