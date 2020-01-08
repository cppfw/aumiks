#pragma once

#include "Source.hpp"

namespace aumiks{

class NullSource : public Source{
public:
	bool fillSampleBuffer(utki::span<Frame> buf)noexcept override;
};

}