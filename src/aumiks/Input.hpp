#pragma once

#include <utki/SpinLock.hpp>
#include <mutex>

#include "Exc.hpp"
#include "Source.hpp"
#include "Frame.hpp"

namespace aumiks{



template <class T_Sample> class Input{
protected:
	std::shared_ptr<aumiks::Source<T_Sample>> src;
	
	utki::SpinLock spinLock;
	
	Input(){}
public:
	virtual ~Input()noexcept{}
	
	virtual audout::Frame_e frameType()const noexcept = 0;
	
	void disconnect()noexcept{
		//To minimize the time with locked spinlock need to avoid object destruction
		//within the locked spinlock period. To achieve that use temporary strong reference outside of spinlock.
		std::shared_ptr<aumiks::Source<T_Sample>> tmp;

		if (this->src) {
			std::lock_guard<utki::SpinLock> guard(this->spinLock);
			tmp = this->src;
			tmp->isConnected_v = false;
			this->src.reset();
		}
	}
	
	void connect(std::shared_ptr<aumiks::Source<T_Sample>> source);
	
	bool isConnected()const{
		return this->src.operator bool();
	}
};



template <class T_Sample, audout::Frame_e frame_type> class FramedInput : public Input<T_Sample>{
	std::shared_ptr<FramedSource<T_Sample, frame_type>> srcInUse;

public:

	FramedInput(){}

	audout::Frame_e frameType() const noexcept override{
		return frame_type;
	}

	bool fillSampleBuffer(utki::Buf<Frame<T_Sample, frame_type>> buf)noexcept{
		if(this->src != std::static_pointer_cast<aumiks::Source<T_Sample>>(this->srcInUse)){
			std::lock_guard<utki::SpinLock> guard(this->spinLock);
			ASSERT(!this->src || this->src->frameType() == frame_type)
			this->srcInUse = std::dynamic_pointer_cast<typename decltype(srcInUse)::element_type>(this->src);
		}
		if(!this->srcInUse){
			return false;
		}
		return this->srcInUse->fillSampleBuffer(buf);
	}
};


template <class T_Sample, audout::Frame_e from_type, audout::Frame_e to_type> class Reframer :
		public FramedSource<T_Sample, to_type>,
		public SingleInputSource<T_Sample>
	{
	
	void fillSampleBuffer_i(utki::Buf<Frame<T_Sample, to_type>> buf)noexcept{
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
								dst->channel[0] = (src->channel[0] + src->channel[1]) / 2;
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
	
	std::vector<Frame<T_Sample, from_type>> tmpBuf;
	
	FramedInput<T_Sample, from_type> input_v;
public:
	Input<T_Sample> & input()override{
		return this->input_v;
	}
	
	bool fillSampleBuffer(utki::Buf<Frame<T_Sample, to_type>> buf)noexcept override{
		if(this->tmpBuf.size() != buf.size()){
			this->tmpBuf.resize(buf.size());
		}
		
		bool ret = this->input_v.fillSampleBuffer(utki::wrapBuf(this->tmpBuf));
		
		this->fillSampleBuffer_i(buf);
		
		return ret;
	}
	
};

template <class T_Sample> void Input<T_Sample>::connect(std::shared_ptr<aumiks::Source<T_Sample> > source){
	ASSERT(source)

	if (this->isConnected()) {
		throw aumiks::Exc("Input already connected");
	}

	if (source->isConnected()) {
		throw aumiks::Exc("Source is already connected");
	}

	if(this->frameType() != source->frameType()){
		std::shared_ptr<SingleInputSource<T_Sample>> r;
		if(source->frameType() == audout::Frame_e::STEREO && this->frameType() == audout::Frame_e::MONO){
			r = utki::makeShared<Reframer<T_Sample, audout::Frame_e::STEREO, audout::Frame_e::MONO>>();
		}else if(source->frameType() == audout::Frame_e::MONO && this->frameType() == audout::Frame_e::STEREO){
			r = utki::makeShared<Reframer<T_Sample, audout::Frame_e::MONO, audout::Frame_e::STEREO>>();
		}else{
			throw Exc("Unimplemented!!! Reframer for requested frame types is not implemented yet!");
		}
		ASSERT(r)
		r->input().connect(source);
		source = std::move(r);
	}

	{
		std::lock_guard<utki::SpinLock> guard(this->spinLock);
		source->isConnected_v = true;
		this->src = std::move(source);
	}
}

}