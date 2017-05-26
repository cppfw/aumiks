#pragma once

#include "Source.hpp"

namespace aumiks{

template <class T_Sample, audout::Frame_e frame_type> class NullSource : public FramedSource<T_Sample, frame_type>{
public:
	bool fillSampleBuffer(utki::Buf<Frame<T_Sample, frame_type>> buf)noexcept override{
		for(auto& f : buf){
			for(auto& c : f.channel){
				c = 0;
			}
		}
		return false;
	}
};

}