#include "Resampler.hpp"

using namespace aumiks;

bool Resampler::fillSampleBuffer(utki::span<Frame> buf) noexcept{
	ASSERT(this->step > 0) //if step is 0 then there will be infinite loop

			//variable step can be changed from another thread, so copy it here
			typename std::remove_volatile<decltype(this->step)>::type s = this->step;

	auto dst = buf.begin();

	if (s > DScale) {//if up-sampling
		size_t filledFromPrevCall = 0;
		//something has left from previous call
		for (; this->scale > 0 && dst != buf.end(); scale -= DScale, ++dst, ++filledFromPrevCall) {
			*dst = this->lastFrameForUpsampling;
		}
		if (dst == buf.end()) {
			return false;
		}
		this->tmpBuf.resize((buf.size() - filledFromPrevCall) * DScale / s + 1);
	} else {
		this->tmpBuf.resize((buf.size()) * DScale / s);
	}

	bool ret = this->input.fillSampleBuffer(utki::make_span(this->tmpBuf));

	auto src = this->tmpBuf.cbegin();
	for (; dst != buf.end(); ++src) {
		this->scale += s;
		for (; this->scale > 0 && dst != buf.end(); this->scale -= DScale, ++dst) {
			ASSERT_INFO(dst != buf.end(),
					"s = " << s <<
					" buf.size() = " << buf.size() <<
					" this->tmpBuf.size() = " << this->tmpBuf.size() <<
					" this->scale = " << this->scale <<
					" dst-end = " << (dst - buf.end())
					)
					ASSERT_INFO(src != this->tmpBuf.cend(),
					"s = " << s <<
					" buf.size() = " << buf.size() <<
					" this->tmpBuf.size() = " << this->tmpBuf.size() <<
					" this->scale = " << this->scale <<
					" dst-end = " << (dst - buf.end())
					)
					* dst = *src;
		}
	}
	ASSERT(dst == buf.end())

	if (src != this->tmpBuf.cend()) {
		//one more sample left in source buffer
		ASSERT_INFO(src + 1 == this->tmpBuf.cend(),
				"s = " << s <<
				" buf.size() = " << buf.size() <<
				" this->tmpBuf.size() = " << this->tmpBuf.size() <<
				" this->scale = " << this->scale <<
				" src-end = " << (src - this->tmpBuf.cend())
				)
				this->scale += s;
	}

	if (this->scale > 0) {
		ASSERT(s > DScale) //was upsampling
				this->lastFrameForUpsampling = this->tmpBuf.back();
	}

	return ret;
}
