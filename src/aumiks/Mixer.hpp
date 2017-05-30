#pragma once

#include "Source.hpp"
#include "Input.hpp"

#include <list>

#include <utki/SpinLock.hpp>

namespace aumiks{

template <class T_Sample> class Mixer : virtual public Source<T_Sample>{
	volatile bool isFinite_v = true;
	
protected:
	utki::SpinLock spinLock;
	
	virtual Input<T_Sample>& add() = 0;
	
public:
	template <class T> void connect(std::shared_ptr<T> source){
		std::lock_guard<decltype(this->spinLock)> guard(this->spinLock);
		this->add().connect(std::move(source));
	}
	
	void setFinite(bool finite)noexcept{
		this->isFinite_v = finite;
	}
	
	bool isFinite()const noexcept{
		return this->isFinite_v;
	}
};


template <class T_Sample, audout::Frame_e frame_type> class FramedMixer :
		public FramedSource<T_Sample, frame_type>,
		public Mixer<T_Sample>
{
	std::list<FramedInput<T_Sample, frame_type>> inputs;
	
	decltype(inputs) inputsToAdd;
	
	std::vector<Frame<T_Sample, frame_type>> tmpBuf;
	
	Input<T_Sample>& add() override{
		this->inputsToAdd.emplace_back();
		return this->inputsToAdd.back();
	}
	
public:
	FramedMixer(){}
	
	FramedMixer(const FramedMixer&) = delete;
	FramedMixer& operator=(const FramedMixer&) = delete;
	
	bool fillSampleBuffer(utki::Buf<Frame<T_Sample, frame_type>> buf)noexcept override{
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
			
			for(auto dst = buf.begin(), end = buf.end(), src = this->tmpBuf.cbegin();
					dst != end;
					++dst, ++src
				)
			{
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
