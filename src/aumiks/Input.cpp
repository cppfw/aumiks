#include "Input.hpp"


using namespace aumiks;



void Input::connect(std::shared_ptr<aumiks::Source> source) {
	ASSERT(source)

	if (this->isConnected()) {
		throw aumiks::Exc("Input already connected");
	}

	if (source->isConnected()) {
		throw aumiks::Exc("Source is already connected");
	}

	ASSERT(audout::AudioFormat::numChannels(this->frameType()) == source->numChannels())
	//TODO: if channels are not equal
			
	{
		std::lock_guard<utki::SpinLock> guard(this->spinLock);
		source->isConnected_var = true;
		this->src = std::move(source);
	}
}


void Input::disconnect() noexcept{
	//To minimize the time with locked spinlock need to avoid object destruction
	//within the locked spinlock period. To achieve that use temporary strong reference.
	std::shared_ptr<aumiks::Source> tmp;

	if (this->src) {
		std::lock_guard<utki::SpinLock> guard(this->spinLock);
		tmp = this->src;
		tmp->isConnected_var = false;
		this->src.reset();
	}
}
