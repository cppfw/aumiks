/*
The MIT License (MIT)

Copyright (c) 2011-2021 Ivan Gagis <igagis@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

/* ================ LICENSE END ================ */

#include <papki/fs_file.hpp>

#include <utki/shared.hpp>

#include "WavSound.hpp"
#include "Resampler.hpp"

using namespace aumiks;

namespace{
template <class TSampleType, audout::frame frame_type>
		class WavSoundImpl : public WavSound
{
	std::vector<TSampleType> data;
	
	class Source : public aumiks::Source{
		const std::shared_ptr<const WavSoundImpl> wavSound;
		
		size_t curSmp = 0;
	
	public:
		Source(std::shared_ptr<const WavSoundImpl> sound) :
				wavSound(std::move(sound))
		{
			ASSERT(this->wavSound)
		}

	private:
		bool fillSampleBuffer(utki::span<frame> buf)noexcept override{
			ASSERT(this->wavSound->data.size() % audout::num_channels(frame_type) == 0)
			ASSERT(this->curSmp % audout::num_channels(frame_type) == 0)
			
			size_t framesToCopy = (this->wavSound->data.size() - this->curSmp) / audout::num_channels(frame_type);
			using std::min;
			framesToCopy = min(framesToCopy, buf.size()); // clamp top

			ASSERT(framesToCopy <= buf.size())
			
			if(framesToCopy == 0){
				// TODO: check number of replays
				return true;
			}

			ASSERT(this->curSmp <= this->wavSound->data.size())			
			const TSampleType *startSmp = &this->wavSound->data[this->curSmp];
			
			this->curSmp += framesToCopy * audout::num_channels(frame_type);
			
			auto dst = buf.begin();
			for(const TSampleType *src = startSmp; dst != buf.begin() + framesToCopy; ++dst){
				unsigned i = 0;
				for(; i != audout::num_channels(frame_type); ++i, ++src){
					dst->channel[i] = real(*src);
				}
				for(; i != audout::num_channels(audout::frame::stereo); ++i){
					dst->channel[i] = real(0);
				}
			}

			// fill the rest with zeroes
			for(; dst != buf.end(); ++dst){
				for(auto& c : dst->channel){
					c = real(0);
				}
			}
			return false;
		}
	};

private:
	std::shared_ptr<aumiks::Source> createSource(uint32_t samplingRate = 0)const override{
		auto src = std::make_shared<Source>(utki::make_shared_from(*this));
		if(samplingRate == 0 || samplingRate == this->samplingRate){
			return src;
		}
		
		auto resampler = std::make_shared<Resampler>();
		
		resampler->input.connect(std::move(src));
		
		resampler->setScale(this->samplingRate, samplingRate);
		
		return resampler;
	}
	
public:
	// assume that data in d is little-endian
	WavSoundImpl(const utki::span<uint8_t> d, uint32_t frequency) :
			WavSound(audout::num_channels(audout::frame::stereo), frequency)
	{
		ASSERT(d.size() % (this->numChannels * sizeof(TSampleType)) == 0)

		this->data.resize(d.size() / sizeof(TSampleType));

		const std::uint8_t* src = d.begin();
		auto dst = this->data.begin();
		for(; src != d.end(); ++dst){
			TSampleType tmp = 0;
			for(unsigned i = 0; i != sizeof(TSampleType); ++i){
				ASSERT(d.begin() <= src && src < d.end())
				tmp |= ((TSampleType(*src)) << (8 * i));
				++src;
			}
			ASSERT(this->data.begin() <= dst && dst < this->data.end())
			*dst = tmp;
		}
	}
};
}

std::shared_ptr<WavSound> WavSound::load(const std::string& fileName){
	papki::fs_file fi(fileName);
	return WavSound::load(fi);
}

std::shared_ptr<WavSound> WavSound::load(papki::file& fi){
	papki::file::guard fileGuard(fi, papki::file::mode::read); // make sure we close the file even in case of exception is thrown

	// Start reading Wav-file header
	{
		std::array<std::uint8_t, 4> riff;
		fi.read(utki::make_span(riff)); // Read 'RIFF' signature
		if(std::string(reinterpret_cast<char*>(&*riff.begin()), riff.size()) != "RIFF"){
			throw std::invalid_argument("WavSound::LoadWAV(): 'RIFF' signature not found");
		}
	}

	fi.seek_forward(4); // Skip "Wav-file size minus 7". We are not interested in this information

	{
		std::array<std::uint8_t, 4> wave;
		fi.read(utki::make_span(wave)); // Read 'WAVE' signature
		if(std::string(reinterpret_cast<char*>(&*wave.begin()), wave.size()) != "WAVE"){
			throw std::invalid_argument("WavSound::LoadWAV(): 'WAVE' signature not found");
		}
	}

	{
		std::array<std::uint8_t, 4> fmt;
		fi.read(utki::make_span(fmt)); // Read 'fmt ' signature
		if(std::string(reinterpret_cast<char*>(&*fmt.begin()), fmt.size()) != "fmt "){
			throw std::invalid_argument("WavSound::LoadWAV(): 'fmt ' signature not found");
		}
	}

	fi.seek_forward(4); // Skip 4 bytes. Their purpose is unknown to me.

	unsigned chans;
	{
		std::array<std::uint8_t, 4> pcmBuf;
		fi.read(utki::make_span(pcmBuf));
		std::uint32_t pcm = utki::deserialize32le(&*pcmBuf.begin());
		if((pcm & 0x0000ffff) != 1){ // Low word indicates whether the file is in PCM format
			throw std::invalid_argument("C_PCM_NonStreamedSound::LoadWAV(): not a PCM format, only PCM format is supported");
		}

		chans = unsigned(pcm >> 16); // High word contains the number of channels (1 for mono, 2 for stereo)
		if(chans != 1 && chans != 2){
			// only mono or stereo is supported and nothing other
			throw std::invalid_argument("WavSound::LoadWAV(): unsupported number of channels");
		}
	}

	// Read in the sound quantization frequency
	std::uint32_t frequency;
	{
		std::array<std::uint8_t, 4> buf;
		fi.read(utki::make_span(buf));
		frequency = utki::deserialize32le(&*buf.begin());
	}

	fi.seek_forward(4); // Playback speed (freq * PCMSampleSize). We don't need this info.

	std::uint32_t bitDepth;
	{
		std::array<std::uint8_t, 4> buf;
		fi.read(utki::make_span(buf));
		bitDepth = utki::deserialize32le(&*buf.begin());
		bitDepth >>= 16; // High word contains the sound bit depth
	}

	{
		std::array<std::uint8_t, 4> data;
		fi.read(utki::make_span(data)); // Read 'data' signature
		if(std::string(reinterpret_cast<char*>(&*data.begin()), data.size()) != "data"){
			throw std::invalid_argument("WavSound::LoadWAV(): 'data' signature not found");
		}
	}

	std::uint32_t dataSize;
	{
		std::array<std::uint8_t, 4> buf;
		fi.read(utki::make_span(buf)); // read the size of the sound data
		dataSize = utki::deserialize32le(&*buf.begin());
	}
	
	// read in the sound data
	std::vector<std::uint8_t> data(dataSize);
	{
		auto bytesRead = fi.read(utki::make_span(data)); // Load Sound data

		if(bytesRead != size_t(dataSize)){
			throw std::invalid_argument("WavSound::LoadWAV(): sound data size is incorrect");
		}
	}
	
	// Now we have Wav-file info
	std::shared_ptr<WavSound> ret;
	if(bitDepth == 8){
//		C_Ref<C_PCM_ParticularNonStreamedSound<s8> > r = new C_PCM_ParticularNonStreamedSound<s8>(chans, cppfw::uint(frequency), dataSize);
//		ret = static_cast<C_PCM_NonStreamedSound*>(r.operator->());
//		bytesRead = fi.read(r->buf.Buf(), r->buf.SizeOfArray());//Load Sound data
//		//convert data to signed format
//		for(s8* ptr=r->buf.Buf(); ptr<(r->buf.Buf()+r->buf.SizeOfArray()); ++ptr)
//			*ptr=s8(int(*ptr)-0x80);
		// TODO: support it
		throw std::invalid_argument("WavSound::LoadWAV(): unsupported bit depth (8 bit) wav file (TODO:)");
	}else if(bitDepth == 16){
		// set the format
		switch(chans){
			case 1: // mono
				ret = std::make_shared<WavSoundImpl<std::int16_t, audout::frame::mono>>(utki::make_span(data), frequency);
				break;
			case 2: // stereo
				ret = std::make_shared<WavSoundImpl<std::int16_t, audout::frame::stereo>>(utki::make_span(data), frequency);
				break;
			default:
				throw std::invalid_argument("WavSound::LoadWAV():  unsupported number of channels");
		}
	}else{
		throw std::invalid_argument("WavSound::LoadWAV(): unsupported bit depth");
	}

	return ret;
}


