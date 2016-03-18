/**
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once

#include <utki/Shared.hpp>
#include <utki/Buf.hpp>

#include <audout/AudioFormat.hpp>


namespace aumiks{


//TODO: doxygen
class Source : virtual public utki::Shared{
	friend class Input;
	
	bool isConnected = false;
	
	audout::AudioFormat::EFrame frameType_var;
	
	std::uint32_t frequency_var;
	
protected:
	Source(const Source&) = delete;
	Source& operator=(const Source&) = delete;
	
	Source(decltype(frameType_var) frameType, std::uint32_t frequency) :
			frameType_var(frameType),
			frequency_var(frequency)
	{}
public:
	
	virtual ~Source()noexcept{}
	
	decltype(frameType_var) frameType()const noexcept{
		return this->frameType_var;
	}
	
	decltype(frequency_var) frequency()const noexcept{
		return this->frequency_var;
	}
	
	unsigned NumChannels()const noexcept{
		return audout::AudioFormat::numChannels(this->frameType_var);
	}
	
	//thread safe
	bool IsConnected()const noexcept{
		return this->isConnected;
	}
private:

};


template <audout::AudioFormat::EFrame frame_type> class ChanneledSource : public Source{
public:
	ChanneledSource(const ChanneledSource&) = delete;
	ChanneledSource& operator=(const ChanneledSource&) = delete;
	
	ChanneledSource(std::uint32_t frequency) :
			Source(frame_type, frequency)
	{}
	
	virtual bool fillSampleBuffer(utki::Buf<std::int32_t> buf)noexcept = 0;
};


}
