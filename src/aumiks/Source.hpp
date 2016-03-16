/**
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once

#include <utki/Shared.hpp>

#include "Output.hpp"

namespace aumiks{


//TODO: doxygen
class Source : virtual public utki::Shared{
	friend class Input;
	
	Source(const Source&);
	Source& operator=(const Source&);
	
	bool isConnected = false;
	
protected:
	Source(aumiks::Output& output) :
			output(output)
	{}
public:
	aumiks::Output& output;
	
	virtual ~Source()noexcept{}
	
	//thread safe
	bool IsConnected()const noexcept{
		return this->isConnected;
	}
private:

};



}
