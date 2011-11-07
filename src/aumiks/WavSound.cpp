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

#include "WavSound.hpp"


using namespace ting;
using namespace aumiks;



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

	//Now we have Wav-file info
	Ref<WavSound> ret(new WavSound());
	unsigned bytesRead;
	if(bitDepth == 8){
//		C_Ref<C_PCM_ParticularNonStreamedSound<s8> > r = new C_PCM_ParticularNonStreamedSound<s8>(chans, igagis::uint(frequency), dataSize);
//		ret = static_cast<C_PCM_NonStreamedSound*>(r.operator->());
//		bytesRead = fi.Read(r->buf.Buf(), r->buf.SizeOfArray());//Load Sound data
//		//convert data to signed format
//		for(s8* ptr=r->buf.Buf(); ptr<(r->buf.Buf()+r->buf.SizeOfArray()); ++ptr)
//			*ptr=s8(int(*ptr)-0x80);
		ASSERT_INFO(false, "8 bit WAV files are not supported yet")
	}else if(bitDepth == 16){
		//set the format
		if(chans == 1){//mono
			if(frequency == 44100){
				ret->format = WavSound::MONO_44100_S16;
			}else{
				ASSERT_INFO(false, "unsupported WAV file frequency: " << frequency)
			}
		}else{//stereo
			ASSERT(chans == 2)

			if(frequency == 44100){
				ret->format = WavSound::STEREO_44100_S16;
			}else{
				ASSERT_INFO(false, "unsupported WAV file frequency")
			}
		}

		//read in the sound data
		ASSERT(dataSize % 2 == 0)//check that dataSize is even
		ret->data.Init(dataSize);
		bytesRead = fi.Read(ret->data);//Load Sound data
	}else{
		throw Exc("WavSound::LoadWAV(): unsupported bit depth");
	}

	if(bytesRead != dataSize){
		throw Exc("WavSound::LoadWAV(): sound data size is incorrect");
	}

	//M_SOUND_PRINT(<<"C_Sound::LoadWAV(): sound loaded, freq="<<(this->freq)<<" bd="<<(this->bd)<<" chans="<<(this->chans)<<" length="<<this->length<<std::endl)
	return ret;
}


//TODO: remove this
/*
//override
bool WavSound::Channel::MixToMixBuf(Array<s32>& mixBuf){
	if(this->stopFlag)
		return true;
	
	switch(ASS(this->wavSound)->format){
		case WavSound::STEREO_44100_S16:
			return this->MixStereo44100S16ToMixBuf(mixBuf);
			break;
		case WavSound::MONO_44100_S16:
			return this->MixMono44100S16ToMixBuf(mixBuf);
			break;
		default:
			ASSERT_INFO(false, "unsupported PCM data format")
			break;
	}//~switch(format)
	
	return true;
}
*/


bool WavSound::Channel::MixMono44100S16ToMixBuf(ting::Buffer<ting::s32>& mixBuf){
	ASSERT(this->wavSound->data.Size() % 2 == 0)
	ASSERT(this->curPos < this->wavSound->data.Size())
	ASSERT(this->curPos % 2 == 0)//make sure curPos is not corrupted (we work with 16 bit samples, mono)
	const s16* src = reinterpret_cast<const s16*>(&this->wavSound->data[this->curPos]);

	s32* dst = mixBuf.Begin();

	u32 samplesCopied = 0;

	//if sound end will be reached
	{
		u32 samplesTillSoundEnd = (this->wavSound->data.Size() - this->curPos) / 2;
		ASSERT(mixBuf.Size() % 2 == 0)
		if(samplesTillSoundEnd <= mixBuf.Size() / 2){
			for(u32 i = 0; i < samplesTillSoundEnd; ++i){
				s32 tmp = s32(*src);
				*dst += tmp;
				++dst;
				*dst += tmp;
				++dst;
				++src;
			}
			this->curPos = 0;

			samplesCopied = samplesTillSoundEnd;

			//TODO: if sound is looped then continue
			return true;//remove channel from playing pool
		}
	}

	for(u32 i = samplesCopied; i < mixBuf.Size() / 2; ++i){
		s32 tmp = s32(*src);
		*dst += tmp;
		++dst;
		*dst += tmp;
		++dst;
		++src;
	}
	this->curPos += (mixBuf.Size() / 2 - samplesCopied) * 2;
	return false;
}



bool WavSound::Channel::MixStereo44100S16ToMixBuf(ting::Buffer<ting::s32>& mixBuf){
	if(this->stopFlag)
		return true;
	
	ASSERT(this->wavSound->data.Size() % 4 == 0)
	ASSERT(this->curPos < this->wavSound->data.Size())
	ASSERT(this->curPos % 4 == 0)//make sure curPos is not corrupted (we work with 16 bit samples, stereo)
	const s16* src = reinterpret_cast<const s16*>(&this->wavSound->data[this->curPos]);

	s32* dst = mixBuf.Begin();

	u32 samplesCopied = 0;

	//if sound end will be reached
	{
		u32 samplesTillSoundEnd = (this->wavSound->data.Size() - this->curPos) / 2;
		if(samplesTillSoundEnd <= mixBuf.Size()){
			for(u32 i = 0; i < samplesTillSoundEnd; ++i){
				*dst += s32(*src);
				++dst;
				++src;
			}
			this->curPos = 0;

			samplesCopied = samplesTillSoundEnd;

			//TODO: if sound is looped then continue
			return true;//remove channel from playing pool
		}
	}

	for(u32 i = samplesCopied; i < mixBuf.Size(); ++i){
		*dst += s32(*src);
		++dst;
		++src;
	}
	this->curPos += (mixBuf.Size() - samplesCopied) * 2;
	return false;
}

