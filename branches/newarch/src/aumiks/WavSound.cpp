/* The MIT License:

Copyright (c) 2009-2012 Ivan Gagis

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE. */

// Home page: http://aumiks.googlecode.com



#include <ting/Buffer.hpp>
#include <ting/fs/FSFile.hpp>

#include "WavSound.hpp"


using namespace ting;
using namespace aumiks;




namespace{

//puts sound frame to output sample buffer.
//NOTE: partial specialization of a function template is not allowed by C++, therefore use static class member
template <class TSampleType, unsigned chans, unsigned freq, unsigned outputChans, unsigned outputFreq>
		struct FrameToSmpBufPutter
{
	static inline void Put(const TSampleType*& src, ting::s32*& dst);
};



//========== Mono 11025 output

template <class TSampleType> struct FrameToSmpBufPutter<TSampleType, 1, 11025, 1, 11025>{
	static inline void Put(const TSampleType*& src, ting::s32*& dst){
		*dst = ting::s32(*src);
		++dst;
		++src;
	}
};

template <class TSampleType> struct FrameToSmpBufPutter<TSampleType, 2, 11025, 1, 11025>{
	static inline void Put(const TSampleType*& src, ting::s32*& dst){
		ting::s32 tmp = ting::s32(*src);
		++src;
		tmp += ting::s32(*src);
		++src;
		tmp /= 2;
		
		*dst = tmp;
		++dst;
	}
};

template <class TSampleType> struct FrameToSmpBufPutter<TSampleType, 1, 22050, 1, 11025>{
	static inline void Put(const TSampleType*& src, ting::s32*& dst){
		FrameToSmpBufPutter<TSampleType, 2, 11025, 1, 11025>::Put(src, dst);
	}
};

template <class TSampleType> struct FrameToSmpBufPutter<TSampleType, 2, 22050, 1, 11025>{
	static inline void Put(const TSampleType*& src, ting::s32*& dst){
		ting::s32 tmp = ting::s32(*src);
		++src;
		tmp += ting::s32(*src);
		++src;
		tmp += ting::s32(*src);
		++src;
		tmp += ting::s32(*src);
		++src;
		tmp /= 4;
		
		*dst = tmp;
		++dst;
	}
};

template <class TSampleType> struct FrameToSmpBufPutter<TSampleType, 1, 44100, 1, 11025>{
	static inline void Put(const TSampleType*& src, ting::s32*& dst){
		FrameToSmpBufPutter<TSampleType, 2, 22050, 1, 11025>::Put(src, dst);
	}
};

template <class TSampleType> struct FrameToSmpBufPutter<TSampleType, 2, 44100, 1, 11025>{
	static inline void Put(const TSampleType*& src, ting::s32*& dst){
		ting::s32 tmp = ting::s32(*src);
		++src;
		tmp += ting::s32(*src);
		++src;
		tmp += ting::s32(*src);
		++src;
		tmp += ting::s32(*src);
		++src;
		tmp += ting::s32(*src);
		++src;
		tmp += ting::s32(*src);
		++src;
		tmp += ting::s32(*src);
		++src;
		tmp += ting::s32(*src);
		++src;
		
		tmp /= 8;
		
		*dst = tmp;
		++dst;
	}
};



//========== Stereo 11025 output

template <class TSampleType> struct FrameToSmpBufPutter<TSampleType, 1, 11025, 2, 11025>{
	static inline void Put(const TSampleType*& src, ting::s32*& dst){
		*dst = ting::s32(*src);
		++dst;
		*dst = ting::s32(*src);
		++dst;
		
		++src;
	}
};

template <class TSampleType> struct FrameToSmpBufPutter<TSampleType, 2, 11025, 2, 11025>{
	static inline void Put(const TSampleType*& src, ting::s32*& dst){
		*dst = s32(*src);
		++dst;
		++src;
		*dst = s32(*src);
		++dst;
		++src;
	}
};

template <class TSampleType> struct FrameToSmpBufPutter<TSampleType, 1, 22050, 2, 11025>{
	static inline void Put(const TSampleType*& src, ting::s32*& dst){
		s32 tmp = ting::s32(*src);
		++src;
		tmp += ting::s32(*src);
		++src;
		tmp /= 2;
		
		*dst = tmp;
		++dst;
		*dst = tmp;
		++dst;
	}
};

template <class TSampleType> struct FrameToSmpBufPutter<TSampleType, 2, 22050, 2, 11025>{
	static inline void Put(const TSampleType*& src, ting::s32*& dst){
		ting::s32 tmp1 = ting::s32(*src);
		++src;
		ting::s32 tmp2 = ting::s32(*src);
		++src;
		tmp1 += ting::s32(*src);
		++src;
		tmp2 += ting::s32(*src);
		++src;
		tmp1 /= 2;
		tmp2 /= 2;
		
		*dst = tmp1;
		++dst;
		*dst = tmp2;
		++dst;
	}
};

template <class TSampleType> struct FrameToSmpBufPutter<TSampleType, 1, 44100, 2, 11025>{
	static inline void Put(const TSampleType*& src, ting::s32*& dst){
		s32 tmp = s32(*src);
		++src;
		tmp += s32(*src);
		++src;
		tmp += s32(*src);
		++src;
		tmp += s32(*src);
		++src;
		tmp /= 4;
		
		*dst = tmp;
		++dst;
		*dst = tmp;
		++dst;
	}
};

template <class TSampleType> struct FrameToSmpBufPutter<TSampleType, 2, 44100, 2, 11025>{
	static inline void Put(const TSampleType*& src, ting::s32*& dst){
		s32 tmp1 = s32(*src);
		++src;
		s32 tmp2 = s32(*src);
		++src;
		tmp1 += s32(*src);
		++src;
		tmp2 += s32(*src);
		++src;
		tmp1 += s32(*src);
		++src;
		tmp2 += s32(*src);
		++src;
		tmp1 += s32(*src);
		++src;
		tmp2 += s32(*src);
		++src;
		tmp1 /= 4;
		tmp2 /= 4;
		
		*dst = tmp1;
		++dst;
		*dst = tmp2;
		++dst;
	}
};



//========== Mono 22050 output

template <class TSampleType> struct FrameToSmpBufPutter<TSampleType, 1, 11025, 1, 22050>{
	static inline void Put(const TSampleType*& src, ting::s32*& dst){
		FrameToSmpBufPutter<TSampleType, 1, 11025, 2, 11025>::Put(src, dst);
	}
};

template <class TSampleType> struct FrameToSmpBufPutter<TSampleType, 2, 11025, 1, 22050>{
	static inline void Put(const TSampleType*& src, ting::s32*& dst){
		FrameToSmpBufPutter<TSampleType, 1, 22050, 2, 11025>::Put(src, dst);
	}
};

template <class TSampleType> struct FrameToSmpBufPutter<TSampleType, 1, 22050, 1, 22050>{
	static inline void Put(const TSampleType*& src, ting::s32*& dst){
		FrameToSmpBufPutter<TSampleType, 1, 11025, 1, 11025>::Put(src, dst);
	}
};

template <class TSampleType> struct FrameToSmpBufPutter<TSampleType, 2, 22050, 1, 22050>{
	static inline void Put(const TSampleType*& src, ting::s32*& dst){
		FrameToSmpBufPutter<TSampleType, 2, 11025, 1, 11025>::Put(src, dst);
	}
};

template <class TSampleType> struct FrameToSmpBufPutter<TSampleType, 1, 44100, 1, 22050>{
	static inline void Put(const TSampleType*& src, ting::s32*& dst){
		FrameToSmpBufPutter<TSampleType, 2, 22050, 1, 22050>::Put(src, dst);
	}
};

template <class TSampleType> struct FrameToSmpBufPutter<TSampleType, 2, 44100, 1, 22050>{
	static inline void Put(const TSampleType*& src, ting::s32*& dst){
		FrameToSmpBufPutter<TSampleType, 2, 22050, 1, 11025>::Put(src, dst);
	}
};



//========== Stereo 22050 output

template <class TSampleType> struct FrameToSmpBufPutter<TSampleType, 1, 11025, 2, 22050>{
	static inline void Put(const TSampleType*& src, ting::s32*& dst){
		*dst = ting::s32(*src);
		++dst;
		*dst = ting::s32(*src);
		++dst;
		*dst = ting::s32(*src);
		++dst;
		*dst = ting::s32(*src);
		++dst;
		
		++src;
	}
};

template <class TSampleType> struct FrameToSmpBufPutter<TSampleType, 2, 11025, 2, 22050>{
	static inline void Put(const TSampleType*& src, ting::s32*& dst){
		s32 tmp = s32(*src);
		++src;
		
		*dst = tmp;
		++dst;
		*dst = s32(*src);
		++dst;
		
		*dst = tmp;
		++dst;
		*dst = s32(*src);
		++dst;
		
		++src;
	}
};

template <class TSampleType> struct FrameToSmpBufPutter<TSampleType, 1, 22050, 2, 22050>{
	static inline void Put(const TSampleType*& src, ting::s32*& dst){
		FrameToSmpBufPutter<TSampleType, 1, 11025, 2, 11025>::Put(src, dst);
	}
};

template <class TSampleType> struct FrameToSmpBufPutter<TSampleType, 2, 22050, 2, 22050>{
	static inline void Put(const TSampleType*& src, ting::s32*& dst){
		FrameToSmpBufPutter<TSampleType, 2, 11025, 2, 11025>::Put(src, dst);
	}
};

template <class TSampleType> struct FrameToSmpBufPutter<TSampleType, 1, 44100, 2, 22050>{
	static inline void Put(const TSampleType*& src, ting::s32*& dst){
		FrameToSmpBufPutter<TSampleType, 1, 22050, 2, 11025>::Put(src, dst);
	}
};

template <class TSampleType> struct FrameToSmpBufPutter<TSampleType, 2, 44100, 2, 22050>{
	static inline void Put(const TSampleType*& src, ting::s32*& dst){
		FrameToSmpBufPutter<TSampleType, 2, 22050, 2, 11025>::Put(src, dst);
	}
};



//========== Mono 44100 output

template <class TSampleType> struct FrameToSmpBufPutter<TSampleType, 1, 11025, 1, 44100>{
	static inline void Put(const TSampleType*& src, ting::s32*& dst){
		FrameToSmpBufPutter<TSampleType, 1, 11025, 2, 22050>::Put(src, dst);
	}
};

template <class TSampleType> struct FrameToSmpBufPutter<TSampleType, 2, 11025, 1, 44100>{
	static inline void Put(const TSampleType*& src, ting::s32*& dst){
		s32 tmp = s32(*src);
		++src;
		tmp += s32(*src);
		tmp /= 2;
		++src;
		
		*dst = tmp;
		++dst;
		*dst = tmp;
		++dst;
		*dst = tmp;
		++dst;
		*dst = tmp;
		++dst;
	}
};

template <class TSampleType> struct FrameToSmpBufPutter<TSampleType, 1, 22050, 1, 44100>{
	static inline void Put(const TSampleType*& src, ting::s32*& dst){
		FrameToSmpBufPutter<TSampleType, 1, 11025, 2, 11025>::Put(src, dst);
	}
};

template <class TSampleType> struct FrameToSmpBufPutter<TSampleType, 2, 22050, 1, 44100>{
	static inline void Put(const TSampleType*& src, ting::s32*& dst){
		FrameToSmpBufPutter<TSampleType, 1, 22050, 2, 11025>::Put(src, dst);
	}
};

template <class TSampleType> struct FrameToSmpBufPutter<TSampleType, 1, 44100, 1, 44100>{
	static inline void Put(const TSampleType*& src, ting::s32*& dst){
		FrameToSmpBufPutter<TSampleType, 1, 11025, 1, 11025>::Put(src, dst);
	}
};

template <class TSampleType> struct FrameToSmpBufPutter<TSampleType, 2, 44100, 1, 44100>{
	static inline void Put(const TSampleType*& src, ting::s32*& dst){
		FrameToSmpBufPutter<TSampleType, 2, 11025, 1, 11025>::Put(src, dst);
	}
};



//========== Stereo 44100 output

template <class TSampleType> struct FrameToSmpBufPutter<TSampleType, 1, 11025, 2, 44100>{
	static inline void Put(const TSampleType*& src, ting::s32*& dst){
		*dst = ting::s32(*src);
		++dst;
		*dst = ting::s32(*src);
		++dst;
		*dst = ting::s32(*src);
		++dst;
		*dst = ting::s32(*src);
		++dst;
		*dst = ting::s32(*src);
		++dst;
		*dst = ting::s32(*src);
		++dst;
		*dst = ting::s32(*src);
		++dst;
		*dst = ting::s32(*src);
		++dst;
		
		++src;
	}
};

template <class TSampleType> struct FrameToSmpBufPutter<TSampleType, 2, 11025, 2, 44100>{
	static inline void Put(const TSampleType*& src, ting::s32*& dst){
		s32 tmp = s32(*src);
		++src;
		
		*dst = tmp;
		++dst;
		*dst = s32(*src);
		++dst;
		
		*dst = tmp;
		++dst;
		*dst = s32(*src);
		++dst;
		
		*dst = tmp;
		++dst;
		*dst = s32(*src);
		++dst;

		*dst = tmp;
		++dst;
		*dst = s32(*src);
		++dst;

		++src;
	}
};

template <class TSampleType> struct FrameToSmpBufPutter<TSampleType, 1, 22050, 2, 44100>{
	static inline void Put(const TSampleType*& src, ting::s32*& dst){
		FrameToSmpBufPutter<TSampleType, 1, 11025, 2, 22050>::Put(src, dst);
	}
};

template <class TSampleType> struct FrameToSmpBufPutter<TSampleType, 2, 22050, 2, 44100>{
	static inline void Put(const TSampleType*& src, ting::s32*& dst){
		FrameToSmpBufPutter<TSampleType, 2, 11025, 2, 22050>::Put(src, dst);
	}
};

template <class TSampleType> struct FrameToSmpBufPutter<TSampleType, 1, 44100, 2, 44100>{
	static inline void Put(const TSampleType*& src, ting::s32*& dst){
		FrameToSmpBufPutter<TSampleType, 1, 11025, 2, 11025>::Put(src, dst);
	}
};

template <class TSampleType> struct FrameToSmpBufPutter<TSampleType, 2, 44100, 2, 44100>{
	static inline void Put(const TSampleType*& src, ting::s32*& dst){
		FrameToSmpBufPutter<TSampleType, 2, 11025, 2, 11025>::Put(src, dst);
	}
};



template <class TSampleType, unsigned chans, unsigned freq> class WavSoundImpl : public WavSound{
	Array<TSampleType> data;
	
	//    ==============================
	//  ==================================
	//=============class Channel============
	class Channel : public WavSound::Channel{
		friend class WavSoundImpl;

		const Ref<const WavSoundImpl> wavSound;
	protected:

	private:
		inline Channel(const Ref<const WavSoundImpl>& sound) :
				wavSound(ASS(sound))
		{}

	private:
		//override
		virtual bool FillSmpBuf11025Mono16(ting::Buffer<ting::s32>& mixBuf){
			return WavSoundImpl::FillSmpBuf<1, 11025>(this, mixBuf);
		}

		//override
		virtual bool FillSmpBuf11025Stereo16(ting::Buffer<ting::s32>& mixBuf){
			return WavSoundImpl::FillSmpBuf<2, 11025>(this, mixBuf);
		}

		//override
		virtual bool FillSmpBuf22050Mono16(ting::Buffer<ting::s32>& mixBuf){
			return WavSoundImpl::FillSmpBuf<1, 22050>(this, mixBuf);
		}

		//override
		virtual bool FillSmpBuf22050Stereo16(ting::Buffer<ting::s32>& mixBuf){
			return WavSoundImpl::FillSmpBuf<2, 22050>(this, mixBuf);
		}

		//override
		virtual bool FillSmpBuf44100Mono16(ting::Buffer<ting::s32>& mixBuf){
			return WavSoundImpl::FillSmpBuf<1, 44100>(this, mixBuf);
		}
		
		//override
		virtual bool FillSmpBuf44100Stereo16(ting::Buffer<ting::s32>& mixBuf){
			return WavSoundImpl::FillSmpBuf<2, 44100>(this, mixBuf);
		}
		
	public:
		static inline ting::Ref<Channel> New(const Ref<const WavSoundImpl>& sound){
			return ting::Ref<Channel>(new Channel(sound));
		}
	};//~class Channel

private:
	//NOTE: local classes are not allowed to have template members by C++ standard, this is why this method is
	//      defined here as static, instead of being a member of Channel class
	template <unsigned outputChans, unsigned outputFreq> static bool FillSmpBuf(Channel* ch, ting::Buffer<ting::s32>& buf){
		ASSERT(buf.Size() % outputChans == 0)

		ASSERT(ch->wavSound->data.Size() % chans == 0)
		ASSERT(ch->curPos < ch->wavSound->data.Size())
		ASSERT(ch->curPos % chans == 0)//make sure curPos is not corrupted

		const TSampleType* src = &ch->wavSound->data[ch->curPos];

		s32* dst = buf.Begin();

		for(;;){
			ASSERT(buf.Begin() <= dst && dst <= buf.End() - 1)
			unsigned samplesTillEndOfBuffer = buf.End() - dst;
			ASSERT(samplesTillEndOfBuffer % outputChans == 0)
			unsigned framesTillEndOfBuffer = samplesTillEndOfBuffer / outputChans;

			ASSERT(ch->wavSound->data.Begin() <= src && src <= ch->wavSound->data.End() - 1)
			
			unsigned samplesTillEndOfSound = ch->wavSound->data.End() - src;
			ASSERT(samplesTillEndOfSound % chans == 0)
			unsigned framesTillEndOfSound =  samplesTillEndOfSound / chans;
			
			unsigned bufFramesTillEndOfSound;
			if(outputFreq >= freq){//rely on optimizer to optimize this 'if' out since it evaluates to constant expression upon template instantiation
				//NOTE: we support only 11025, 22050 and 44100 Hz, so expect ratio of 1, 2 or 4 only
				ASSERT((outputFreq / freq == 1) || (outputFreq / freq == 2) || (outputFreq / freq == 4))
				bufFramesTillEndOfSound = framesTillEndOfSound * (outputFreq / freq);
			}else{
				ASSERT((freq / outputFreq == 1) || (freq / outputFreq == 2) || (freq / outputFreq == 4))
//				ASSERT(framesTillEndOfSound % (freq / outputFreq) == 0)//make sure there is a proper granularity
				bufFramesTillEndOfSound = framesTillEndOfSound / (freq / outputFreq);
				//NOTE: this is the number of integral frames. Because the source sound is of higher
				//      sampling rate than playback we might have to take into account the
				//      fraction of a playback frame later.
			}

//			TRACE(<< "framesTillEndOfSound = " << framesTillEndOfSound << std::endl)
//			TRACE(<< "bufFramesTillEndOfSound = " << bufFramesTillEndOfSound << std::endl)
//			TRACE(<< "framesTillEndOfBuffer = " << framesTillEndOfBuffer << std::endl)
				
			//if sound end will be reached
			if(bufFramesTillEndOfSound < framesTillEndOfBuffer){
				ting::s32* end = dst + (bufFramesTillEndOfSound * outputChans);
				for(; dst != end;){
					ASSERT(buf.Begin() <= dst && dst <= buf.End() - 1)
					ASSERT(ch->wavSound->data.Begin() <= src && src <= ch->wavSound->data.End() - 1)
					FrameToSmpBufPutter<TSampleType, chans, freq, outputChans, outputFreq>::Put(src, dst);
				}
				ch->curPos = 0;
				
				//NOTE: in cases when outputFreq is below freq we might need to deal with sound tail
				//      if it does not match the full frame of output buffer.
				
				//Hope that in cases when this buffer is is not used (freq / outputFreq == 0), it should be optimized out by compiler
				ting::StaticBuffer<TSampleType, (freq / outputFreq) * chans > tail;
				TSampleType* tailIter = tail.Begin();
				
				if(freq / outputFreq != 0){//constant expression, should be optimized out by compiler where necessary
					for(; src != ch->wavSound->data.End(); ++src, ++tailIter){
						ASSERT(tail.Size() > 0)
						ASSERT(tail.Begin() <= tailIter && tailIter <= tail.End() - 1)
						ASSERT(ch->wavSound->data.Begin() <= src && src <= ch->wavSound->data.End() - 1)
						*tailIter = *src;
					}
				}else{
					ASSERT(tail.Size() == 0)
				}
				
				if(ch->numLoops == 1){
					//this was the last repeat of playing
					
					if(freq / outputFreq != 0){//constant expression, should be optimized out by compiler where necessary
						if(tailIter != tail.Begin()){//if we have copied something to the tail buffer
							for(; tailIter != tail.End(); ++tailIter){
								ASSERT(tail.Size() > 0)
								ASSERT(tail.Begin() <= tailIter && tailIter <= tail.End() - 1)
								*tailIter = 0;//fill the rest of sound tail with silence
							}

							const TSampleType *s = tail.Begin();
							ASSERT(buf.Begin() <= dst && dst <= buf.End() - 1)
							FrameToSmpBufPutter<TSampleType, chans, freq, outputChans, outputFreq>::Put(s, dst);
							ASSERT((buf.Begin() <= dst && dst <= buf.End() - 1) || dst == buf.End())
						}
					}
					
					//fill the rest of the sample buffer with zeros
					for(; dst != buf.End(); ++dst){
						ASSERT(buf.Begin() <= dst && dst <= buf.End() - 1)
						*dst = 0;
					}
					ASSERT(ch->curPos == 0)
					ASSERT(ch->numLoops == 1)//Leaving ch->numLoops in default state
					return true;
				}else{
					//if repeating
					
					//decrement loops counter if needed
					if(ch->numLoops != 0){
						ASSERT(ch->numLoops > 1)
						--ch->numLoops;
					}
					
					if(freq / outputFreq != 0){//constant expression, should be optimized out by compiler where necessary
						if(tailIter != tail.Begin()){//if we have copied something to the tail buffer
							//fill the rest of the tail with data from beginning of the sound
							do{
								src = ch->wavSound->data.Begin();
								ASSERT(ch->curPos == 0)

								for(; tailIter != tail.End() && src != ch->wavSound->data.End(); ++src, ++tailIter){
									ASSERT(tail.Size() > 0)
									ASSERT(tail.Begin() <= tailIter && tailIter <= tail.End() - 1)
									ASSERT(ch->wavSound->data.Begin() <= src && src <= ch->wavSound->data.End() - 1)
									*tailIter = *src;
									++ch->curPos;
								}
							}while(tailIter != tail.End());

							const TSampleType *s = tail.Begin();
							ASSERT(buf.Begin() <= dst && dst <= buf.End() - 1)
							FrameToSmpBufPutter<TSampleType, chans, freq, outputChans, outputFreq>::Put(s, dst);
						}else{
							src = ch->wavSound->data.Begin();
							ASSERT(ch->curPos == 0)
						}
					}else{
						ASSERT(tail.Size() == 0)
						src = ch->wavSound->data.Begin();
						ASSERT(ch->curPos == 0)
					}
					continue;//for(;;)
				}
			}else{//no end of sound will be reached
				ASSERT(framesTillEndOfBuffer * outputChans == unsigned(buf.End() - dst))
				for(; dst != buf.End();){
					ASSERT(buf.Begin() <= dst && dst <= buf.End() - 1)
					ASSERT(ch->wavSound->data.Begin() <= src && src <= ch->wavSound->data.End() - 1)
					FrameToSmpBufPutter<TSampleType, chans, freq, outputChans, outputFreq>::Put(src, dst);
				}
				
				ch->curPos += framesTillEndOfBuffer * chans * freq / outputFreq;
				
				ASSERT(ch->curPos < ch->wavSound->data.Size())
				return false;
			}
		}//~for(;;)
		ASSERT(false)
	}
	//=============class Channel============
	//  ==================================
	//    ==============================
	
	//override
	virtual Ref<WavSound::Channel> CreateWavChannel()const{
		return Channel::New(Ref<const WavSoundImpl>(this));
	}
	
private:
	//NOTE: assume that data in d is little-endian
	WavSoundImpl(const ting::Buffer<ting::u8>& d){
		ASSERT(d.Size() % (chans * sizeof(TSampleType)) == 0)

		this->data.Init(d.Size() / sizeof(TSampleType));

		const ting::u8* src = d.Begin();
		TSampleType* dst = this->data.Begin();
		for(; src != d.End(); ++dst){
			TSampleType tmp = 0;
			for(unsigned i = 0; i != sizeof(TSampleType); ++i){
				ASSERT(d.Begin() <= src && src < d.End())
				tmp |= ((TSampleType(*src)) << (8 * i));
				++src;
			}
			ASSERT(this->data.Begin() <= dst && dst < this->data.End())
			*dst = tmp;
		}
	}

public:
	static inline ting::Ref<WavSoundImpl> New(const ting::Buffer<ting::u8>& d){
		return ting::Ref<WavSoundImpl>(new WavSoundImpl(d));
	}
};



}//~namespace



Ref<WavSound> WavSound::LoadWAV(const std::string& fileName){
	ting::fs::FSFile fi(fileName);
	return WavSound::LoadWAV(fi);
}



Ref<WavSound> WavSound::LoadWAV(ting::fs::File& fi){
	ting::fs::File::Guard fileGuard(fi, ting::fs::File::READ);//make sure we close the file even in case of exception is thrown

	//Start reading Wav-file header
	{
		StaticBuffer<u8, 4> riff;
		fi.Read(riff);//Read 'RIFF' signature
		if(std::string(reinterpret_cast<char*>(riff.Begin()), riff.Size()) != "RIFF"){
			throw Exc("WavSound::LoadWAV(): 'RIFF' signature not found");
		}
	}

	fi.SeekForward(4);//Skip "Wav-file size minus 7". We are not intrested in this information

	{
		StaticBuffer<u8, 4> wave;
		fi.Read(wave);//Read 'WAVE' signature
		if(std::string(reinterpret_cast<char*>(wave.Begin()), wave.Size()) != "WAVE"){
			throw Exc("WavSound::LoadWAV(): 'WAVE' signature not found");
		}
	}

	{
		StaticBuffer<u8, 4> fmt;
		fi.Read(fmt);//Read 'fmt ' signature
		if(std::string(reinterpret_cast<char*>(fmt.Begin()), fmt.Size()) != "fmt "){
			throw Exc("WavSound::LoadWAV(): 'fmt ' signature not found");
		}
	}

	fi.SeekForward(4);//Skip 4 bytes. Their purpose is unknown to me.

	unsigned chans;
	{
		StaticBuffer<u8, 4> pcmBuf;
		fi.Read(pcmBuf);
		u32 pcm = ting::util::Deserialize32LE(pcmBuf.Begin());
		if((pcm & 0x0000ffff) != 1){//Low word indicates whether the file is in PCM format
			throw Exc("C_PCM_NonStreamedSound::LoadWAV(): not a PCM format, only PCM format is supported");
		}

		chans = unsigned(pcm >> 16);//High word contains the number of channels (1 for mono, 2 for stereo)
		if(chans != 1 && chans != 2){
			//only mono or stereo is supported and nothing other
			throw Exc("WavSound::LoadWAV(): unsupported number of channels");
		}
	}

	//Read in the sound quantization frequency
	u32 frequency;
	{
		StaticBuffer<u8, 4> buf;
		fi.Read(buf);
		frequency = ting::util::Deserialize32LE(buf.Begin());
	}

	fi.SeekForward(4);//Playback speed (freq * PCMSampleSize). We don't need this info.

	u32 bitDepth;
	{
		StaticBuffer<u8, 4> buf;
		fi.Read(buf);
		bitDepth = ting::util::Deserialize32LE(buf.Begin());
		bitDepth >>= 16;//High word contains the sound bit depth
	}

	{
		StaticBuffer<u8, 4> data;
		fi.Read(data);//Read 'data' signature
		if(std::string(reinterpret_cast<char*>(data.Begin()), data.Size()) != "data"){
			throw Exc("WavSound::LoadWAV(): 'data' signature not found");
		}
	}

	u32 dataSize;
	{
		StaticBuffer<u8, 4> buf;
		fi.Read(buf);//read the size of the sound data
		dataSize = ting::util::Deserialize32LE(buf.Begin());
	}
	
	//read in the sound data
	ting::Array<ting::u8> data(dataSize);
	{
		unsigned bytesRead = fi.Read(data);//Load Sound data

		if(bytesRead != dataSize){
			throw Exc("WavSound::LoadWAV(): sound data size is incorrect");
		}
	}
	
	//Now we have Wav-file info
	Ref<WavSound> ret;
	if(bitDepth == 8){
//		C_Ref<C_PCM_ParticularNonStreamedSound<s8> > r = new C_PCM_ParticularNonStreamedSound<s8>(chans, igagis::uint(frequency), dataSize);
//		ret = static_cast<C_PCM_NonStreamedSound*>(r.operator->());
//		bytesRead = fi.Read(r->buf.Buf(), r->buf.SizeOfArray());//Load Sound data
//		//convert data to signed format
//		for(s8* ptr=r->buf.Buf(); ptr<(r->buf.Buf()+r->buf.SizeOfArray()); ++ptr)
//			*ptr=s8(int(*ptr)-0x80);
		//TODO: support it
		throw Exc("WavSound::LoadWAV(): unsupported bit depth (8 bit) wav file");
	}else if(bitDepth == 16){
		//set the format
		switch(chans){
			case 1://mono
				switch(frequency){
					case 11025:
						ret = WavSoundImpl<ting::s16, 1, 11025>::New(data);
						break;
					case 22050:
						ret = WavSoundImpl<ting::s16, 1, 22050>::New(data);
						break;
					case 44100:
						ret = WavSoundImpl<ting::s16, 1, 44100>::New(data);
						break;
					default:
						throw Exc("WavSound::LoadWAV(): unsupported sampling rate");
				}
				break;
			case 2://stereo
				switch(frequency){
					case 11025:
						ret = WavSoundImpl<ting::s16, 2, 11025>::New(data);
						break;
					case 22050:
						ret = WavSoundImpl<ting::s16, 2, 22050>::New(data);
						break;
					case 44100:
						ret = WavSoundImpl<ting::s16, 2, 44100>::New(data);
						break;
					default:
						throw Exc("WavSound::LoadWAV(): unsupported sampling rate");
				}
				break;
			default:
				throw aumiks::Exc("WavSound::LoadWAV():  unsupported number of channels");
		}//~switch(chans)
	}else{
		throw Exc("WavSound::LoadWAV(): unsupported bit depth");
	}

	//M_SOUND_PRINT(<<"C_Sound::LoadWAV(): sound loaded, freq="<<(this->freq)<<" bd="<<(this->bd)<<" chans="<<(this->chans)<<" length="<<this->length<<std::endl)
	return ret;
}


