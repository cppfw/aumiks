#pragma once

#include "Source.hpp"
#include "Input.hpp"

#include <list>

#include <utki/SpinLock.hpp>

namespace aumiks{

class Mixer : virtual public Source{
	volatile bool isFinite_v = true;
public:
	virtual void connect(std::shared_ptr<Source> source) = 0;
	
	void setFinite(bool finite)noexcept{
		this->isFinite_v = finite;
	}
	
	bool isFinite()const noexcept{
		return this->isFinite_v;
	}
};


template <audout::Frame_e frame_type> class FramedMixer :
		public FramedSource<frame_type>,
		public Mixer
{
	std::list<FramedInput<frame_type>> inputs;
	
	utki::SpinLock spinLock;
	
	decltype(inputs) inputsToAdd;
	
	std::vector<Frame<frame_type>> tmpBuf;
	
public:
	FramedMixer(){}
	
	FramedMixer(const FramedMixer&) = delete;
	FramedMixer& operator=(const FramedMixer&) = delete;
	
	void connect(std::shared_ptr<Source> source) override{
		std::lock_guard<decltype(this->spinLock)> guard(this->spinLock);
		this->inputsToAdd.emplace_back();
		this->inputsToAdd.back().connect(std::move(source));
	}
	
	bool fillSampleBuffer(utki::Buf<Frame<frame_type>> buf)noexcept override{
		{
			std::lock_guard<decltype(this->spinLock)> guard(this->spinLock);
			this->inputs.splice(this->inputs.end(), this->inputsToAdd);
		}
		
		this->tmpBuf.resize(buf.size());
		
		for(auto& f : buf){
			for(auto& c : f.channel){
				c = 0;
			}
		}
		
		for(auto i = this->inputs.begin(); i != this->inputs.end();){
			if(i->fillSampleBuffer(utki::wrapBuf(this->tmpBuf))){
				i = this->inputs.erase(i);
			}else{
				++i;
			}
			
			auto dst = buf.begin();
			auto src = this->tmpBuf.cbegin();
			for(; dst != buf.end(); ++dst, ++src){
				ASSERT(src != this->tmpBuf.cend())
				dst->add(*src);
			}
		}
		
		if(this->isFinite()){
			return this->inputs.size() == 0;
		}else{
			return false;
		}
	}

private:
};

}
