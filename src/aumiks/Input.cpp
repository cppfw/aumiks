#include "Input.hpp"
#include "Reframer.hpp"
#include "Resampler.hpp"

using namespace aumiks;



void Input::connect(std::shared_ptr<aumiks::Source> source) {
	ASSERT(source)

	if (this->isConnected()) {
		throw aumiks::Exc("Input already connected");
	}

	if (source->isConnected()) {
		throw aumiks::Exc("Source is already connected");
	}

	if(this->frameType() != source->frameType()){
		if(source->frameType() == audout::AudioFormat::EFrame::STEREO && this->frameType() == audout::AudioFormat::EFrame::MONO){
			auto r = utki::makeShared<Reframer<audout::AudioFormat::EFrame::STEREO, audout::AudioFormat::EFrame::MONO>>();
			r->input.connect(source);
			source = std::move(r);
		}else if(source->frameType() == audout::AudioFormat::EFrame::MONO && this->frameType() == audout::AudioFormat::EFrame::STEREO){
			auto r = utki::makeShared<Reframer<audout::AudioFormat::EFrame::MONO, audout::AudioFormat::EFrame::STEREO>>();
			r->input.connect(source);
			source = std::move(r);
		}else{
			throw Exc("Unimplemented!!! Reframer for requested frame types is not implemented yet!");
		}
	}
			
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
