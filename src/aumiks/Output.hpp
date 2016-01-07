/**
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once

#include "Exc.hpp"


namespace aumiks{



class Output{
	template <std::uint8_t> friend class ChanOutput;
	
	std::uint8_t numChannels;
	
	Output(const Output&);
	Output& operator=(const Output&);
	
	Output(std::uint8_t numChannels) :
			numChannels(numChannels)
	{}
	
public:
	virtual ~Output()throw(){}
	
	std::uint8_t NumChannels()const throw(){
		return this->numChannels;
	}
};



template <std::uint8_t num_channels> class ChanOutput : public Output{
	ChanOutput(const ChanOutput&);
	ChanOutput& operator=(const ChanOutput&);
	
public:
	
	ChanOutput() :
			Output(num_channels)
	{}
	
	virtual bool FillSampleBuffer(utki::Buf<std::int32_t> buf)noexcept = 0;
};


}
