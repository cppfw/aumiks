/* 
 * File:   SlowdownResampler.hpp
 * Author: ivan
 *
 * Created on January 2, 2014, 12:05 AM
 */

#pragma once

#include "../Source.hpp"
#include "../Input.hpp"

namespace aumiks{

class SlowdownResampler : public Source{
public:
	SlowdownResampler();
	SlowdownResampler(const SlowdownResampler& orig);
	virtual ~SlowdownResampler();
private:

};

}
