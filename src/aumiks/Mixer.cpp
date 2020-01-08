#include "Mixer.hpp"

using namespace aumiks;

void Mixer::connect(std::shared_ptr<Source> source) {
	std::lock_guard<decltype(this->spinLock) > guard(this->spinLock);
	this->inputsToAdd.emplace_back();
	this->inputsToAdd.back().connect(std::move(source));
}

bool Mixer::fillSampleBuffer(utki::span<Frame> buf) noexcept{
	{
		std::lock_guard<decltype(this->spinLock) > guard(this->spinLock);
		this->inputs.splice(this->inputs.end(), this->inputsToAdd);
	}

	this->tmpBuf.resize(buf.size());

	for (auto& f : buf) {
		for (auto& c : f.channel) {
			c = 0;
		}
	}

	for (auto i = this->inputs.begin(); i != this->inputs.end();) {
		if (i->fillSampleBuffer(utki::make_span(this->tmpBuf))) {
			i = this->inputs.erase(i);
		} else {
			++i;
		}

		auto src = this->tmpBuf.cbegin();
		for (auto dst = buf.begin(), end = buf.end();
				dst != end;
				++dst, ++src
				) {
			ASSERT(src != this->tmpBuf.cend())
			dst->add(*src);
		}
	}

	if (this->isFinite()) {
		return this->inputs.size() == 0;
	} else {
		return false;
	}
}
