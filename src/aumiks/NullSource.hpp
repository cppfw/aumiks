#pragma once

#include "Source.hpp"

namespace aumiks{

template <audout::Frame_e frame_type> class NullSource : public FramedSource<frame_type>{
public:
	bool fillSampleBuffer(utki::Buf<Frame<frame_type>> buf)noexcept override{
		for(auto& f : buf){
			for(auto& c : f.channel){
				c = real(0);
			}
		}
		return false;
	}
};

}