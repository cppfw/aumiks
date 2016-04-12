#include "Input.hpp"
#include "Reframer.hpp"
#include "Resampler.hpp"

using namespace aumiks;



void Input::connect(std::shared_ptr<aumiks::ASource> source) {
	ASSERT(source)

	if (this->isConnected()) {
		throw aumiks::Exc("Input already connected");
	}

	if (source->isConnected()) {
		throw aumiks::Exc("Source is already connected");
	}

	if(this->frameType() != source->frameType()){
		std::shared_ptr<AReframer> r;
		if(source->frameType() == audout::Frame_e::STEREO && this->frameType() == audout::Frame_e::MONO){
			r = utki::makeShared<Reframer<audout::Frame_e::STEREO, audout::Frame_e::MONO>>();
		}else if(source->frameType() == audout::Frame_e::MONO && this->frameType() == audout::Frame_e::STEREO){
			r = utki::makeShared<Reframer<audout::Frame_e::MONO, audout::Frame_e::STEREO>>();
		}else{
			throw Exc("Unimplemented!!! Reframer for requested frame types is not implemented yet!");
		}
		ASSERT(r)
		r->input().connect(source);
		source = std::move(r);
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
	std::shared_ptr<aumiks::ASource> tmp;

	if (this->src) {
		std::lock_guard<utki::SpinLock> guard(this->spinLock);
		tmp = this->src;
		tmp->isConnected_var = false;
		this->src.reset();
	}
}
