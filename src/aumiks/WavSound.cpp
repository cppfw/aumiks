/* The MIT License:

Copyright (c) 2009-2011 Ivan Gagis

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


#include <ting/Buffer.hpp>
#include <ting/FSFile.hpp>

#include "WavSound.hpp"


using namespace ting;
using namespace aumiks;




namespace{

//mixes sound frame to mixing buffer.
//NOTE: partial specialization of a function template is not allowed by C++, therefore use static class member
template <class TSampleType, unsigned chans, unsigned freq, unsigned outputChans, unsigned outputFreq>
		struct FrameToSmpBufPutter
{
	static inline void Put(const TSampleType*& src, ting::s32*& dst);
};

template <class TSampleType> struct FrameToSmpBufPutter<TSampleType, 1, 44100, 2, 44100>{
	static inline void Put(const TSampleType*& src, ting::s32*& dst){
		*dst = s32(*src);
		++dst;
		*dst = s32(*src);
		++dst;
		++src;
	}
};

template <class TSampleType> struct FrameToSmpBufPutter<TSampleType, 2, 44100, 2, 44100>{
	static inline void Put(const TSampleType*& src, ting::s32*& dst){
		*dst = s32(*src);
		++dst;
		++src;
		*dst = s32(*src);
		++dst;
		++src;
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

		ting::Inited<unsigned, 0> curPos;//current index in samples into sound data buffer
	protected:

	private:
		inline Channel(const Ref<const WavSoundImpl>& sound) :
				wavSound(ASS(sound))
		{}

	private:
		//TODO: uncomment
/*
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
 */
		
		//override
		virtual bool FillSmpBuf44100Stereo16(ting::Buffer<ting::s32>& mixBuf){
			return WavSoundImpl::FillSmpBuf<2, 44100>(this, mixBuf);
		}
		
	public:
		static inline ting::Ref<Channel> New(const Ref<const WavSoundImpl>& sound){
			return ting::Ref<Channel>(new Channel(sound));
		}
	};//~class Channel
	
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
			
			if(outputFreq >= freq){//rely on optimizer to optimize this if out since it evaluates to constant expression upon template instantiation
				unsigned samplesTillEndOfSound = ch->wavSound->data.End() - src;
				ASSERT(samplesTillEndOfSound % chans == 0)
				unsigned framesTillEndOfSound =  samplesTillEndOfSound / chans;
				
				//NOTE: we support only 11025, 22050 and 44100 Hz, so expect ratio of 1, 2 or 4 only
				ASSERT((outputFreq / freq == 1) || (outputFreq / freq == 2) || (outputFreq / freq == 4))
				
				unsigned bufFramesTillEndOfSound = framesTillEndOfSound * (outputFreq / freq);

//				TRACE(<< "framesTillEndOfSound = " << framesTillEndOfSound << std::endl)
//				TRACE(<< "bufFramesTillEndOfSound = " << bufFramesTillEndOfSound << std::endl)
//				TRACE(<< "framesTillEndOfBuffer = " << framesTillEndOfBuffer << std::endl)
				
				//if sound end will be reached
				if(bufFramesTillEndOfSound <= framesTillEndOfBuffer){
					for(; src != ch->wavSound->data.End();){
						ASSERT(ch->wavSound->data.Begin() <= src && src <= ch->wavSound->data.End() - 1)
						ASSERT(buf.Begin() <= dst && dst <= buf.End() - 1)
						FrameToSmpBufPutter<TSampleType, chans, freq, outputChans, outputFreq>::Put(src, dst);
					}
					if(ch->numLoops > 0){
						--ch->numLoops;
						if(ch->numLoops == 0){
							ch->curPos = 0;
							return true;
						}else{
							continue;
						}
					}else{
						//loop infinitely
						continue;
					}
				}else{//no end of sound will be reached
					for(; dst != buf.End();){
						ASSERT(buf.Begin() <= dst && dst <= buf.End() - 1)
						ASSERT(ch->wavSound->data.Begin() <= src && src <= ch->wavSound->data.End() - 1)
						FrameToSmpBufPutter<TSampleType, chans, freq, outputChans, outputFreq>::Put(src, dst);
					}
					ch->curPos += framesTillEndOfBuffer * chans * freq / outputFreq;
					return false;
				}
				
			}else{//freq < outputFreq
				ASSERT(false)
				//TODO:
				return false;
			}
		}//~for(;;)

		
		
		
		
		
		
		/*
		

		//TODO:!!!!!!!!!!
		
		//if sound end will be reached
		for(;;){
			unsigned samplesTillSoundEnd = (ch->wavSound->data.Size() - ch->curPos);
			if(samplesTillSoundEnd <= (buf.Size() - samplesCopied)){
				for(unsigned i = 0; i < samplesTillSoundEnd; ++i){
					ASSERT(buf.Begin() <= dst && dst <= buf.End() - 1)
					*dst = s32(*src);
					++dst;
					++src;
				}
				ch->curPos = 0;

				samplesCopied += samplesTillSoundEnd;

				//if sound is looped then continue
				if(ch->looped){
					continue;
				}

				//TODO: fill the rest of buffer with zeros (silence)
				return true;//remove channel from playing pool
			}else{
				break;
			}
		}

		for(; dst != buf.End();){
			ASSERT(buf.Begin() <= dst && dst <= buf.End() - 1)
			*dst = s32(*src);
			++dst;
			++src;
		}
		ch->curPos += (buf.Size() - samplesCopied);
		return false;
		*/
	}
	//=============class Channel============
	//  ==================================
	//    ==============================
	
	//override
	virtual Ref<WavSound::Channel> CreateChannel()const{
		return Channel::New(Ref<const WavSoundImpl>(this));
	}
	
private:
	//NOTE: assume that data in d is little-endian
	WavSoundImpl(const ting::Buffer<ting::u8>& d){
		ASSERT(d.Size() % (chans * sizeof(TSampleType)) == 0)

		this->data.Init(d.Size() / sizeof(TSampleType));

		const ting::u8* src = d.Begin();
		for(TSampleType* dst = this->data.Begin(); dst != this->data.End(); ++dst){
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
	ting::FSFile fi(fileName);
	return WavSound::LoadWAV(fi);
}



Ref<WavSound> WavSound::LoadWAV(File& fi){
	File::Guard fileGuard(fi, File::READ);//make sure we close the file even in case of exception is thrown

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
		u32 pcm = ting::Deserialize32(pcmBuf.Begin());
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
		frequency = ting::Deserialize32(buf.Begin());
	}

	fi.SeekForward(4);//Playback speed (freq * PCMSampleSize). We don't need this info.

	u32 bitDepth;
	{
		StaticBuffer<u8, 4> buf;
		fi.Read(buf);
		bitDepth = ting::Deserialize32(buf.Begin());
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
		dataSize = ting::Deserialize32(buf.Begin());
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
		ASSERT_INFO_ALWAYS(false, "8 bit WAV files are not supported yet")
	}else if(bitDepth == 16){
		//set the format
		if(chans == 1){//mono
			if(frequency == 44100){
				ret = WavSoundImpl<ting::s16, 1, 44100>::New(data);
			}else{
				ASSERT_INFO_ALWAYS(false, "unsupported WAV file frequency: " << frequency)
			}
		}else if(chans == 2){//stereo
			if(frequency == 44100){
				ret = WavSoundImpl<ting::s16, 2, 44100>::New(data);
			}else{
				ASSERT_INFO_ALWAYS(false, "unsupported WAV file frequency")
			}
		}else{
			throw aumiks::Exc("WavSound::LoadWAV():  unsupported number of channels");
		}
	}else{
		throw Exc("WavSound::LoadWAV(): unsupported bit depth");
	}

	//M_SOUND_PRINT(<<"C_Sound::LoadWAV(): sound loaded, freq="<<(this->freq)<<" bd="<<(this->bd)<<" chans="<<(this->chans)<<" length="<<this->length<<std::endl)
	return ret;
}


