#pragma once

#include "Source.hpp"

namespace aumiks{

class NullSource : public Source{
public:
	bool fillSampleBuffer(utki::Buf<Frame> buf)noexcept override;
};

}