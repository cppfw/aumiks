/*
The MIT License (MIT)

Copyright (c) 2011-2024 Ivan Gagis <igagis@gmail.com>

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
#include <utki/string.hpp>
#include <utki/util.hpp>

#include "wav_sound.hpp"
#include "resampler.hpp"

using namespace aumiks;

namespace{
template <
		class TSampleType,
		audout::frame frame_type
	>
class wav_sound_impl : public wav_sound
{
	std::vector<TSampleType> data;
	
	class source : public aumiks::source{
		const utki::shared_ref<const wav_sound_impl> sound;
		
		size_t cur_sample = 0;
	
	public:
		source(const utki::shared_ref<const wav_sound_impl>& sound) :
				sound(sound)
		{}

	private:
		bool fill_sample_buffer(utki::span<frame> buf)noexcept override{
			ASSERT(this->sound.get().data.size() % audout::num_channels(frame_type) == 0)
			ASSERT(this->cur_sample % audout::num_channels(frame_type) == 0)
			
			size_t frames_to_copy = (this->sound.get().data.size() - this->cur_sample) / audout::num_channels(frame_type);
			using std::min;
			frames_to_copy = min(frames_to_copy, buf.size()); // clamp top

			ASSERT(frames_to_copy <= buf.size())
			
			if(frames_to_copy == 0){
				// TODO: check number of replays
				return true;
			}

			ASSERT(this->cur_sample <= this->sound.get().data.size())			
			const TSampleType *start_sample = &this->sound.get().data[this->cur_sample];
			
			this->cur_sample += frames_to_copy * audout::num_channels(frame_type);
			
			auto dst = buf.begin();
			for(const TSampleType *src = start_sample; dst != buf.begin() + frames_to_copy; ++dst){
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
	std::shared_ptr<aumiks::source> create_source(uint32_t sampling_rate = 0)const override{
		auto src = std::make_shared<source>(utki::make_shared_from(*this));
		if(sampling_rate == 0 || sampling_rate == this->sampling_rate){
			return src;
		}
		
		auto resampler = std::make_shared<aumiks::resampler>();
		
		resampler->input.connect(std::move(src));
		
		resampler->set_scale(this->sampling_rate, sampling_rate);
		
		return resampler;
	}
	
public:
	// assume that data in d is little-endian
	wav_sound_impl(utki::span<const uint8_t> wav_data, uint32_t frequency) :
			wav_sound(audout::num_channels(audout::frame::stereo), frequency)
	{
		ASSERT(wav_data.size() % (this->num_channels * sizeof(TSampleType)) == 0)

		this->data.resize(wav_data.size() / sizeof(TSampleType));

		const std::uint8_t* src = wav_data.begin();
		auto dst = this->data.begin();
		for(; src != wav_data.end(); ++dst){
			TSampleType tmp = 0;
			for(unsigned i = 0; i != sizeof(TSampleType); ++i){
				ASSERT(wav_data.begin() <= src && src < wav_data.end())
				tmp |= ((TSampleType(*src)) << (8 * i));
				++src;
			}
			ASSERT(this->data.begin() <= dst && dst < this->data.end())
			*dst = tmp;
		}
	}
};
}

std::shared_ptr<wav_sound> wav_sound::load(const std::string& file_name){
	papki::fs_file fi(file_name);
	return wav_sound::load(fi);
}

std::shared_ptr<wav_sound> wav_sound::load(papki::file& fi){
	papki::file::guard file_guard(fi, papki::mode::read); // make sure we close the file even in case of exception is thrown

	// Start reading Wav-file header
	{
		std::array<uint8_t, 4> riff;
		fi.read(riff); // read 'RIFF' signature
		if(utki::make_string_view(riff) != "RIFF"){
			throw std::invalid_argument("wav_sound::load(wav): 'RIFF' signature not found");
		}
	}

	fi.seek_forward(4); // Skip "Wav-file size minus 7". We are not interested in this information

	{
		std::array<uint8_t, 4> wave;
		fi.read(wave); // read 'WAVE' signature
		if(utki::make_string_view(wave) != "WAVE"){
			throw std::invalid_argument("wav_sound::load(wav): 'WAVE' signature not found");
		}
	}

	{
		std::array<uint8_t, 4> fmt;
		fi.read(fmt); // read 'fmt ' signature
		if(utki::make_string_view(fmt) != "fmt "){
			throw std::invalid_argument("wav_sound::load(wav): 'fmt ' signature not found");
		}
	}

	fi.seek_forward(4); // skip 4 bytes, their purpose is unknown to me

	unsigned chans;
	{
		std::array<uint8_t, 4> pcm_buf;
		fi.read(pcm_buf);
		uint32_t pcm = utki::deserialize32le(pcm_buf.data());
		if((pcm & 0x0000ffff) != 1){ // low word indicates whether the file is in PCM format
			throw std::invalid_argument("wav_sound::load(wav): not a PCM format, only PCM format is supported");
		}

		chans = unsigned(pcm >> 16); // high word contains the number of channels (1 for mono, 2 for stereo)
		if(chans != 1 && chans != 2){
			// only mono or stereo is supported and nothing other
			throw std::invalid_argument("wav_sound::load(wav): unsupported number of channels");
		}
	}

	// read in the sound quantization frequency
	uint32_t frequency;
	{
		std::array<uint8_t, 4> buf;
		fi.read(buf);
		frequency = utki::deserialize32le(buf.data());
	}

	fi.seek_forward(4); // Playback speed (freq * PCMSampleSize). We don't need this info.

	uint32_t bit_depth;
	{
		std::array<uint8_t, 4> buf;
		fi.read(buf);
		bit_depth = utki::deserialize32le(buf.data());
		bit_depth >>= 16; // high word contains the sound bit depth
	}

	{
		std::array<uint8_t, 4> data;
		fi.read(data); // read 'data' signature
		if(utki::make_string_view(data) != "data"){
			throw std::invalid_argument("wav_sound::load(wav): 'data' signature not found");
		}
	}

	uint32_t data_size;
	{
		std::array<uint8_t, 4> buf;
		fi.read(buf); // read the size of the sound data
		data_size = utki::deserialize32le(buf.data());
	}
	
	// read in the sound data
	std::vector<uint8_t> data(data_size);
	{
		auto num_bytes_read = fi.read(data); // load Sound data

		if(num_bytes_read != size_t(data_size)){
			throw std::invalid_argument("wav_sound::load(wav): sound data size is incorrect");
		}
	}
	
	// now we have Wav-file info
	std::shared_ptr<wav_sound> ret;
	if(bit_depth == 8){
//		C_Ref<C_PCM_ParticularNonStreamedSound<s8> > r = new C_PCM_ParticularNonStreamedSound<s8>(chans, cppfw::uint(frequency), dataSize);
//		ret = static_cast<C_PCM_NonStreamedSound*>(r.operator->());
//		bytesRead = fi.read(r->buf.Buf(), r->buf.SizeOfArray());//Load Sound data
//		//convert data to signed format
//		for(s8* ptr=r->buf.Buf(); ptr<(r->buf.Buf()+r->buf.SizeOfArray()); ++ptr)
//			*ptr=s8(int(*ptr)-0x80);
		// TODO: support it
		throw std::invalid_argument("wav_sound::load(wav): unsupported bit depth (8 bit) wav file (TODO:)");
	}else if(bit_depth == 16){
		// set the format
		switch(chans){
			case 1: // mono
				ret = std::make_shared<wav_sound_impl<int16_t, audout::frame::mono>>(data, frequency);
				break;
			case 2: // stereo
				ret = std::make_shared<wav_sound_impl<int16_t, audout::frame::stereo>>(data, frequency);
				break;
			default:
				throw std::invalid_argument("wav_sound::load(wav): unsupported number of channels");
		}
	}else{
		throw std::invalid_argument("wav_sound::load(wav): unsupported bit depth");
	}

	return ret;
}
