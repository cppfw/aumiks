#include "Input.hpp"


using namespace aumiks;



void Input::Connect(const std::shared_ptr<aumiks::Source>& source) {
	ASSERT(source)
	ASSERT(audout::AudioFormat::numChannels(this->frameType()) == source->NumChannels())

	if (this->IsConnected()) {
		throw aumiks::Exc("Input already connected");
	}

	if (source->IsConnected()) {
		throw aumiks::Exc("Source is already connected");
	}

	{
		std::lock_guard<utki::SpinLock> guard(this->spinLock);
		source->isConnected = true;
		this->src = source;
	}
}


void Input::Disconnect() noexcept{
	//To minimize the time with locked spinlock need to avoid object destruction
	//within the locked spinlock period. To achieve that use temporary strong reference.
	std::shared_ptr<aumiks::Source> tmp;

	if (this->src) {
		std::lock_guard<utki::SpinLock> guard(this->spinLock);
		tmp = this->src;
		tmp->isConnected = false;
		this->src.reset();
	}
}
