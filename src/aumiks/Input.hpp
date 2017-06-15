#pragma once

#include <utki/SpinLock.hpp>

#include <mutex>

#include <type_traits>

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
	
	template <class T> void connect(std::shared_ptr<T> source){
		this->connectInternal(std::static_pointer_cast<aumiks::Source<typename T::Sample_t>>(source));
	}
	
	bool isConnected()const{
		return this->src.operator bool();
	}
	
private:
	template <class T_SrcSample> void connectInternal(std::shared_ptr<aumiks::Source<T_SrcSample>> source);
	
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

template <class T_To, class T_From, audout::Frame_e frame_type> class Retyper :
		public FramedSource<T_To, frame_type>
	{
	
	FramedInput<T_From, frame_type> input_v;
	
	std::vector<Frame<T_From, frame_type>> tmpBuf;
public:
	Input<T_From> & input(){
		return this->input_v;
	}
	
	bool fillSampleBuffer(utki::Buf<Frame<T_To, frame_type>> buf)noexcept override{
		if(this->tmpBuf.size() != buf.size()){
			this->tmpBuf.resize(buf.size());
		}
		
		bool ret = this->input_v.fillSampleBuffer(utki::wrapBuf(this->tmpBuf));
		
		auto d = buf.begin();
		for(
				auto s = this->tmpBuf.begin(), se = this->tmpBuf.end();
				s != se;
				++s, ++d
			)
		{
			ASSERT_INFO(d != buf.end(), "buf.size() = " << buf.size() << ", this->tmpBuf.size() = " << this->tmpBuf.size())
			auto ss = s->channel.begin(), sse = s->channel.end();
			for(auto dd = d->channel.begin(); ss != sse; ++ss, ++dd){
				ASSERT(dd != d->channel.end())
				*dd = T_To(*ss);
			}
		}
		
		return ret;
	}
	
};



template<class T_Sample, class T_SrcSample>
typename std::enable_if<std::is_same<T_Sample, T_SrcSample>::value, std::shared_ptr<aumiks::Source<T_Sample>>>::type
    retypeInternal(std::shared_ptr<aumiks::Source<T_SrcSample>>& source)
{
    return source;
}

template<class T_Sample, class T_SrcSample>
typename std::enable_if<!std::is_same<T_Sample, T_SrcSample>::value, std::shared_ptr<aumiks::Source<T_Sample>>>::type
    retypeInternal(std::shared_ptr<aumiks::Source<T_SrcSample>>& source)
{
    switch(source->frameType()){
		case audout::Frame_e::MONO:
			{
				auto r = utki::makeShared<Retyper<T_Sample, T_SrcSample, audout::Frame_e::MONO>>();
				r->input().connect(std::move(source));
				return r;
			}
		case audout::Frame_e::STEREO:
			{
				auto r = utki::makeShared<Retyper<T_Sample, T_SrcSample, audout::Frame_e::STEREO>>();
				r->input().connect(std::move(source));
				return r;
			}
		default:
			ASSERT(false)
			return nullptr;
	}
}
 


template <class T_Sample>
template <class T_SrcSample>
void Input<T_Sample>::connectInternal(std::shared_ptr<aumiks::Source<T_SrcSample>> source){
	ASSERT(source)

	if (this->isConnected()) {
		throw aumiks::Exc("Input already connected");
	}

	if (source->isConnected()) {
		throw aumiks::Exc("Source is already connected");
	}

	if(this->frameType() != source->frameType()){
		std::shared_ptr<SingleInputSource<T_SrcSample>> r;
		if(source->frameType() == audout::Frame_e::STEREO && this->frameType() == audout::Frame_e::MONO){
			r = utki::makeShared<Reframer<T_SrcSample, audout::Frame_e::STEREO, audout::Frame_e::MONO>>();
		}else if(source->frameType() == audout::Frame_e::MONO && this->frameType() == audout::Frame_e::STEREO){
			r = utki::makeShared<Reframer<T_SrcSample, audout::Frame_e::MONO, audout::Frame_e::STEREO>>();
		}else{
			throw Exc("Unimplemented!!! Reframer for requested frame types is not implemented yet!");
		}
		ASSERT(r)
		r->input().connect(source);
		source = std::move(r);
	}

	std::shared_ptr<aumiks::Source<T_Sample>> finalSource;
	
	finalSource = retypeInternal<T_Sample, T_SrcSample>(source);
	
	{
		std::lock_guard<utki::SpinLock> guard(this->spinLock);
		finalSource->isConnected_v = true;
		this->src = std::move(finalSource);
	}
}

}
