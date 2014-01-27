/* The MIT License:

Copyright (c) 2009-2014 Ivan Gagis

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
#include <ting/util.hpp>

#include "Exc.hpp"
#include "WavSound.hpp"
#include "Lib.hpp"



using namespace aumiks;



namespace{



template <class TSampleType, unsigned channels, ting::u32 frequency, unsigned outputChannels, ting::u32 outputFrequency> class WavSoundImpl : public WavSound{
	ting::Array<TSampleType> data;
	

	class Channel : public WavSound::Channel{
		friend class WavSoundImpl;

		const ting::Ref<const WavSoundImpl> wavSound;
	protected:

	private:
		inline Channel(const ting::Ref<const WavSoundImpl>& sound) :
				wavSound(ASS(sound))
		{}
	public:
		static inline ting::Ref<Channel> New(const ting::Ref<const WavSoundImpl>& sound){
			return ting::Ref<Channel>(new Channel(sound));
		}

	private:
		//override
		virtual bool FillSmpBuf(ting::Buffer<ting::s32>& buf){
			ASSERT(buf.Size() % aumiks::Lib::Inst().OutputFormat().frame.NumChannels() == 0)
			
			ASSERT(this->wavSound->data.Size() % channels == 0)
			ASSERT(this->curSmp % channels == 0)
			
			size_t framesInBuf = buf.Size() / outputChannels;
			
			size_t framesToCopy = (this->wavSound->data.Size() - this->curSmp) / channels;
			ting::util::ClampTop(framesToCopy, framesInBuf);

			ASSERT(framesToCopy <= framesInBuf)
			
			if(framesToCopy == 0){
				//TODO: check number of replays
				return true;
			}

			ASSERT(this->curSmp <= this->wavSound->data.Size())			
			const TSampleType *startSmp = &this->wavSound->data[this->curSmp];
			
			this->curSmp += framesToCopy * channels;
			
			if(channels == outputChannels){
				if(frequency == outputFrequency){
					ting::s32 *dst = buf.Begin();
					for(const TSampleType *src = startSmp; dst != buf.Begin() + framesToCopy * outputChannels; ++dst, ++src){
						*dst = ting::s32(*src);
					}
					
					//fill the rest with zeroes
					for(; dst != buf.End(); ++dst){
						*dst = 0;
					}
					return false;
				}else{
					//TODO: resample
					return true;
				}
			}else if(channels < outputChannels){
				if(frequency == outputFrequency){
					ting::s32 *dst = buf.Begin();
					for(const TSampleType *src = startSmp; dst != buf.Begin() + framesToCopy * outputChannels;){
						unsigned c = 0;
						ting::s32 avg = 0;
						for(; c != channels; ++c, ++dst, ++src){
							*dst = ting::s32(*src);
							avg += *dst;
						}
						for(; c != outputChannels; ++c, ++dst){
							*dst = avg;
						}
					}
					
					for(; dst != buf.End(); ++dst){
						*dst = 0;
					}
					return false;
				}else{
					//TODO: resample
					return true;
				}
			}else{
				// channels > chans
				
				//TODO:
				if(this->wavSound->SamplingRate() == outputFrequency){
					ting::s32 *dst = buf.Begin();
					for(const TSampleType *src = startSmp; dst != buf.Begin() + framesToCopy * outputChannels;){						
						ting::s32 avg = 0;
						for(unsigned c = outputChannels; c != this->wavSound->NumChannels(); ++c){
							avg += ting::s32(src[c]);
						}
						
						for(unsigned c = 0; c != outputChannels; ++c, ++dst, ++src){
							*dst = ting::s32(*src);//(ting::s32(*src) + avg) / ting::s32(this->wavSound->NumChannels() - chans + 1);
						}
					}
					
					for(; dst != buf.End(); ++dst){
						*dst = 0;
					}
					return false;
				}else{
					//TODO: resample
					return true;
				}
			}

			return true;
		}
	};//~class Channel

private:
	//override
	virtual ting::Ref<WavSound::Channel> CreateWavChannel()const{
		return Channel::New(ting::Ref<const WavSoundImpl>(this));
	}
	
private:
	//NOTE: assume that data in d is little-endian
	WavSoundImpl(const ting::Buffer<ting::u8>& d) :
			WavSound(channels, frequency)
	{
		ASSERT(d.Size() % (this->NumChannels() * sizeof(TSampleType)) == 0)

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



ting::Ref<WavSound> WavSound::LoadWAV(const std::string& fileName){
	ting::fs::FSFile fi(fileName);
	return WavSound::LoadWAV(fi);
}



ting::Ref<WavSound> WavSound::LoadWAV(ting::fs::File& fi){
	ting::fs::File::Guard fileGuard(fi, ting::fs::File::READ);//make sure we close the file even in case of exception is thrown

	//Start reading Wav-file header
	{
		ting::StaticBuffer<ting::u8, 4> riff;
		fi.Read(riff);//Read 'RIFF' signature
		if(std::string(reinterpret_cast<char*>(riff.Begin()), riff.Size()) != "RIFF"){
			throw Exc("WavSound::LoadWAV(): 'RIFF' signature not found");
		}
	}

	fi.SeekForward(4);//Skip "Wav-file size minus 7". We are not interested in this information

	{
		ting::StaticBuffer<ting::u8, 4> wave;
		fi.Read(wave);//Read 'WAVE' signature
		if(std::string(reinterpret_cast<char*>(wave.Begin()), wave.Size()) != "WAVE"){
			throw Exc("WavSound::LoadWAV(): 'WAVE' signature not found");
		}
	}

	{
		ting::StaticBuffer<ting::u8, 4> fmt;
		fi.Read(fmt);//Read 'fmt ' signature
		if(std::string(reinterpret_cast<char*>(fmt.Begin()), fmt.Size()) != "fmt "){
			throw Exc("WavSound::LoadWAV(): 'fmt ' signature not found");
		}
	}

	fi.SeekForward(4);//Skip 4 bytes. Their purpose is unknown to me.

	unsigned chans;
	{
		ting::StaticBuffer<ting::u8, 4> pcmBuf;
		fi.Read(pcmBuf);
		ting::u32 pcm = ting::util::Deserialize32LE(pcmBuf.Begin());
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
	ting::u32 frequency;
	{
		ting::StaticBuffer<ting::u8, 4> buf;
		fi.Read(buf);
		frequency = ting::util::Deserialize32LE(buf.Begin());
	}

	fi.SeekForward(4);//Playback speed (freq * PCMSampleSize). We don't need this info.

	ting::u32 bitDepth;
	{
		ting::StaticBuffer<ting::u8, 4> buf;
		fi.Read(buf);
		bitDepth = ting::util::Deserialize32LE(buf.Begin());
		bitDepth >>= 16;//High word contains the sound bit depth
	}

	{
		ting::StaticBuffer<ting::u8, 4> data;
		fi.Read(data);//Read 'data' signature
		if(std::string(reinterpret_cast<char*>(data.Begin()), data.Size()) != "data"){
			throw Exc("WavSound::LoadWAV(): 'data' signature not found");
		}
	}

	ting::u32 dataSize;
	{
		ting::StaticBuffer<ting::u8, 4> buf;
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
	ting::Ref<WavSound> ret;
	if(bitDepth == 8){
//		C_Ref<C_PCM_ParticularNonStreamedSound<s8> > r = new C_PCM_ParticularNonStreamedSound<s8>(chans, igagis::uint(frequency), dataSize);
//		ret = static_cast<C_PCM_NonStreamedSound*>(r.operator->());
//		bytesRead = fi.Read(r->buf.Buf(), r->buf.SizeOfArray());//Load Sound data
//		//convert data to signed format
//		for(s8* ptr=r->buf.Buf(); ptr<(r->buf.Buf()+r->buf.SizeOfArray()); ++ptr)
//			*ptr=s8(int(*ptr)-0x80);
		//TODO: support it
		throw Exc("WavSound::LoadWAV(): unsupported bit depth (8 bit) wav file (TODO:)");
	}else if(bitDepth == 16){
		//set the format
		switch(chans){
			case 1://mono
				switch(frequency){
					case 11025:
						ret = WavSoundImpl<ting::s16, 1, 11025, >::New(data);
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


