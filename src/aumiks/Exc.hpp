#pragma once

// TODO: remove whole file

#include <string>
#include <stdexcept>

namespace aumiks{

class Exc : public std::runtime_error{
public:
	inline Exc(const std::string&  msg) :
			std::runtime_error(msg)
	{}
};

}
