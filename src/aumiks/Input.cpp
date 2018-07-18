#include "Input.hpp"

using namespace aumiks;


namespace{
template <audout::Frame_e from_type, audout::Frame_e to_type> class Reframer :
		public FramedSource<to_type>,
		public SingleInputSource
	{
	
	void fillSampleBuffer_i(utki::Buf<Frame<to_type>> buf)noexcept{
		switch(from_type){
			case audout::Frame_e::MONO:
				switch(to_type){
					case audout::Frame_e::STEREO:
						{
							ASSERT(this->tmpBuf.size() == buf.size())

							auto src = this->tmpBuf.cbegin();
							auto dst = buf.begin();
							for(; dst != buf.end(); ++dst, ++src){
								dst->channel[0] = src->channel[0];
								dst->channel[1] = src->channel[0];
							}
						}
						break;
					default:
						ASSERT(false)
						break;
				}
				break;
			case audout::Frame_e::STEREO:
				switch(to_type){
					case audout::Frame_e::MONO:
						{
							ASSERT(this->tmpBuf.size() == buf.size())

							auto src = this->tmpBuf.cbegin();
							auto dst = buf.begin();
							for(; dst != buf.end(); ++dst, ++src){
								dst->channel[0] = (src->channel[0] + src->channel[1]) / real(2);
							}
						}
						break;
					default:
						ASSERT(false)
						break;
				}
				break;
			default:
				ASSERT(false)
				break;
		}
	}
	
	std::vector<Frame<from_type>> tmpBuf;
	
	FramedInput<from_type> input_v;
public:
	Input & input()override{
		return this->input_v;
	}
	
	bool fillSampleBuffer(utki::Buf<Frame<to_type>> buf)noexcept override{
		if(this->tmpBuf.size() != buf.size()){
			this->tmpBuf.resize(buf.size());
		}
		
		bool ret = this->input_v.fillSampleBuffer(utki::wrapBuf(this->tmpBuf));
		
		this->fillSampleBuffer_i(buf);
		
		return ret;
	}
	
};
}

void Input::connect(std::shared_ptr<aumiks::Source> source){
	ASSERT(source)

	if (this->isConnected()) {
		throw aumiks::Exc("Input already connected");
	}

	if (source->isConnected()) {
		throw aumiks::Exc("Source is already connected");
	}

	if(this->frameType() != source->frameType()){
		std::shared_ptr<SingleInputSource> r;
		if(source->frameType() == audout::Frame_e::STEREO && this->frameType() == audout::Frame_e::MONO){
			r = std::make_shared<Reframer<audout::Frame_e::STEREO, audout::Frame_e::MONO>>();
		}else if(source->frameType() == audout::Frame_e::MONO && this->frameType() == audout::Frame_e::STEREO){
			r = std::make_shared<Reframer<audout::Frame_e::MONO, audout::Frame_e::STEREO>>();
		}else{
			throw Exc("Unimplemented!!! Reframer for requested frame types is not implemented yet!");
		}
		ASSERT(r)
		r->input().connect(source);
		source = std::move(r);
	}
	
	{
		std::lock_guard<decltype(this->mutex)> guard(this->mutex);
		source->isConnected_v = true;
		this->src = std::move(source);
	}
}