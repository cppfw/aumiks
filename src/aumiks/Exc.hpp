/**
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once


#include <string>

#include <utki/Exc.hpp>



namespace aumiks{

class Exc : public utki::Exc{
public:
	inline Exc(const std::string&  msg) :
			utki::Exc(msg)
	{}
};

}//~namespace
