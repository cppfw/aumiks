
/**
 * @author Ivan Gagis <igagis@gmail.com>
 */


#pragma once

#include <cstring>

#include <utki/Buf.hpp>

namespace aumiks{



class Channel;



/**
 * @brief Interface for filling the sample buffer.
 * TODO: write docs about chain of sample buffer fillers.
 */
class SampleBufferFiller{
	friend class aumiks::Channel;
	
	//pointer to the next buffer filler in the chain.
	//If 0 then this is the last one in the chain.
	SampleBufferFiller* next = 0;
	
	volatile bool isOff = false;
	
	bool FillSmpBufInternal(utki::Buf<std::int32_t> buf){
		if(this->isOff){
//			TRACE(<< "SampleBufferFiller::FillSmpBufInternal(): isOff is true: this->isOff = " << this->isOff << std::endl)
			return this->FillSmpBufFromNextByChain(buf);
		}
		
//		TRACE(<< "SampleBufferFiller::FillSmpBufInternal(): calling FillSmpBuf" << std::endl)
		bool ret = this->FillSmpBuf(buf);
//		TRACE(<< "SampleBufferFiller::FillSmpBufInternal(): ret = " << ret << std::endl)
		return ret;
	}
	
protected:
    //TODO: re-wise docs
	/**
	 * @brief This function is called when more data to play is needed.
	 * Override this method in your Channel implementation.
	 * Depending on the selected output format (sampling rate, mono/stereo) the corresponding method is called.
	 * The return value indicates whether the sound has finished playing or not.
	 * Note, that channel playing may continue even if sound has stopped playing, this is
	 * possible if there are any effects added to this channel which keeps playing, for example
	 * an echo effect.
	 * @param buf - the sample buffer to fill with the data to play.
	 * @param freq - sampling rate in Hertz.
	 * @param chans - number of channels (1 = mono, 2 = stereo, etc.).
	 * @return true if sound playing has finished. Returning true will result in that the channel will be removed
	 *         from the pool of playing channels and the contents of the 'buf' after this call will not be played.
	 * @return false otherwise.
	 */
	virtual bool FillSmpBuf(utki::Buf<std::int32_t> buf) = 0;
	
	//TODO: doxygen
	//NOTE: the size of the supplied buf should contain integer number of frames!
	inline bool FillSmpBufFromNextByChain(utki::Buf<std::int32_t>& buf){
//		TRACE(<< "SampleBufferFiller::FillSmpBufFromNextByChain(): this->next = " << this->next << std::endl)
		if(!this->next){
			memset(&*buf.begin(), 0, buf.sizeInBytes());
			return false;
		}
		return this->next->FillSmpBufInternal(buf);
	}
	
public:
	virtual ~SampleBufferFiller()throw(){}
	
	
	//TODO: doxygen
	inline void SwitchOnOff_ts(bool isOff)throw(){
		this->isOff = isOff;
	}
};

}//~namespace
