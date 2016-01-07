/**
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once

#include "Source.hpp"


#include <list>

namespace aumiks{

//TODO: doxygen
template <class T_Sample, std::uint8_t num_channels> class Mixer : public ChanSource<T_Sample, num_channels>{
	Mixer(const Mixer&);
	Mixer& operator=(const Mixer&);
	
	typedef std::list<std::shared_ptr<ChanSource<T_Sample, num_channels> > > T_List;
	typedef T_List::iterator T_Iter;
	
	utki::SpinLock addSpinLock;
	T_List* sourcesPendingAddition;
	T_List* sourcesBeingAdded;
	T_List addList1, addList2;
	
	T_List sources;
	
	std::vector<std::int32_t> smpBuf;
	
	bool isPersistent;
	
	Mixer(bool isPersistent) :
			sourcesPendingAddition(addList1),
			sourcesBeingAdded(addList2),
			isPersistent(isPersistent)
	{}
	
	void MixSmpBufTo(utki::Buf<std::int32_t>& buf){
		ASSERT(this->smpBuf.Size() == buf.Size())

		std::int32_t* src = this->smpBuf.Begin();
		std::int32_t* dst = buf.Begin();

		for(; dst != buf.End(); ++src, ++dst){
			*dst += *src;
		}
	}
	
public:
	bool FillSampleBuffer(utki::Buf<T_Sample> buf)noexcept override{
		ASSERT(buf.Size() % num_channels == 0)
		
		{
			atomic::SpinLock::GuardYield guard(this->addSpinLock);
			if(this->sourcesPendingAddition->size() != 0){
				std::swap(this->sourcesPendingAddition, this->sourcesBeingAdded);
			}
		}
		
		//add pending sources
		while(this->sourcesBeingAdded->size() != 0){
			this->sources.push_back(this->sourcesBeingAdded->pop_back());
		}
		
		//check if this mix channel holds sample buffer of a correct size
		if(this->smpBuf.Size() != buf.Size()){
			this->smpBuf.Init(buf.Size());
		}

		T_Iter i = this->sources.begin();
		
		if(i == this->sources.end()){//if there is no sources to play
			//no any child channels to play initially
			if(!this->isPersistent){
				return true;
			}

			//zero out the sample buffer
			memset(buf.Begin(), 0, buf.SizeInBytes());
			return false;
		}
		
		//the very first channel is not mixed, but simply written to the output buffer
		if((*i)->FillSampleBuffer(buf)){
//TODO: ?
//			(*i)->stoppedFlag = true;
			i = this->sources.erase(i);
		}else{
			++i;
		}

		for(; i != this->sources.end();){
			if((*i)->FillSampleBuffer(this->smpBuf)){
//TODO: ?
//				(*i)->stoppedFlag = true;
				i = this->sources.erase(i);
			}else{
				++i;
			}
			this->MixSmpBufTo(buf);
		}

		return !this->isPersistent && (this->sources.size() == 0);
	}
	
	void AddSource(const std::shared_ptr<ChanSource<T_Sample, num_channels> >& src){
		atomic::SpinLock::GuardYield guard(this->addSpinLock);
		this->sourcesPendingAddition->push_back(src);
	}
	
	static std::shared_ptr<Mixer> New(bool isPersistent = false){
		return std::shared_ptr<Mixer>(new Mixer(isPersistent));
	}
};

}
