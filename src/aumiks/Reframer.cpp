#include "Reframer.hpp"


using namespace aumiks;

namespace aumiks{

template<> void Reframer<audout::Frame_e::MONO, audout::Frame_e::STEREO>::fillSampleBuffer_i(utki::Buf<Frame<audout::Frame_e::STEREO>> buf)noexcept{
	ASSERT(this->tmpBuf.size() == buf.size())
	
	auto src = this->tmpBuf.cbegin();
	auto dst = buf.begin();
	for(; dst != buf.end(); ++dst, ++src){
		dst->channel[0] = src->channel[0];
		dst->channel[1] = src->channel[0];
	}
}

template<> void Reframer<audout::Frame_e::STEREO, audout::Frame_e::MONO>::fillSampleBuffer_i(utki::Buf<Frame<audout::Frame_e::MONO>> buf)noexcept{
	ASSERT(this->tmpBuf.size() == buf.size())
	
	auto src = this->tmpBuf.cbegin();
	auto dst = buf.begin();
	for(; dst != buf.end(); ++dst, ++src){
		dst->channel[0] = (src->channel[0] + src->channel[1]) / 2;
	}
}

}

