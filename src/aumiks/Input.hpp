#pragma once

#include <utki/SpinLock.hpp>

#include <type_traits>
#include <mutex>

#include "config.hpp"
#include "Exc.hpp"
#include "Source.hpp"
#include "Frame.hpp"

namespace aumiks{

class Input{
protected:
	std::shared_ptr<Source> src;
	
	std::shared_ptr<Source> srcInUse;
	
	utki::SpinLock mutex;
	
public:
	Input(){}
	
	virtual ~Input()noexcept{}
	
	void disconnect()noexcept;
	
	void connect(std::shared_ptr<Source> source);
	
	bool isConnected()const{
		return this->src.get() != nullptr;
	}

	bool fillSampleBuffer(utki::Buf<Frame> buf)noexcept;	
};

}
