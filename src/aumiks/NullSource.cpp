#include "NullSource.hpp"

using namespace aumiks;

bool NullSource::fillSampleBuffer(utki::Buf<Frame> buf) noexcept{
	for (auto& f : buf) {
		for (auto& c : f.channel) {
			c = real(0);
		}
	}
	return false;
}
