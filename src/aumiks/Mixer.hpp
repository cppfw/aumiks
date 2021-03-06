#pragma once

#include "Source.hpp"
#include "Input.hpp"

#include <list>

#include <utki/spin_lock.hpp>

namespace aumiks{

class Mixer : virtual public Source{
	volatile bool isFinite_v = true;
	
	utki::spin_lock spinLock;
	
	std::list<Input> inputs;
	
	decltype(inputs) inputsToAdd;
	
	std::vector<Frame> tmpBuf;
	
public:
	void connect(std::shared_ptr<Source> source);
	
	void setFinite(bool finite)noexcept{
		this->isFinite_v = finite;
	}
	
	bool isFinite()const noexcept{
		return this->isFinite_v;
	}
	
protected:
	bool fillSampleBuffer(utki::span<Frame> buf)noexcept override;
};


}
