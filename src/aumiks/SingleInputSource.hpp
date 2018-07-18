#pragma once

#include "Source.hpp"
#include "Input.hpp"

namespace aumiks{

class SingleInputSource : public Source{
public:
	Input input;
};

}
