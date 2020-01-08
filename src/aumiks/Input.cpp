#include "Input.hpp"

using namespace aumiks;



void Input::connect(std::shared_ptr<aumiks::Source> source){
	ASSERT(source)

	if (this->isConnected()) {
		throw aumiks::Exc("Input already connected");
	}

	if (source->isConnected()) {
		throw aumiks::Exc("Source is already connected");
	}
	
	{
		std::lock_guard<decltype(this->mutex)> guard(this->mutex);
		source->isConnected_v = true;
		this->src = std::move(source);
	}
}

void Input::disconnect()noexcept {
	std::lock_guard<decltype(this->mutex) > guard(this->mutex);

	if (this->src) {
		this->src->isConnected_v = false;
		this->src.reset();
	}
}


bool Input::fillSampleBuffer(utki::span<Frame> buf) noexcept{
	{
		std::lock_guard<decltype(this->mutex) > guard(this->mutex);
		if (this->src != this->srcInUse) {
			this->srcInUse = this->src;
		}
	}
	if (!this->srcInUse) {
		for (auto& b : buf) {
			for (auto& c : b.channel) {
				c = 0;
			}
		}
		return false;
	}
	return this->srcInUse->fillSampleBuffer(buf);
}
