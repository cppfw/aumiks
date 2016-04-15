#include <papki/FSFile.hpp>

#include <utki/Shared.hpp>

#include "Exc.hpp"
#include "WavSound.hpp"

#include "Resampler.hpp"


using namespace aumiks;



namespace{



template <class TSampleType, audout::Frame_e frame_type>
		class WavSoundImpl : public WavSound
{
	std::vector<TSampleType> data;
	

	class Source : public aumiks::Source<frame_type>{
//		friend class WavSoundImpl;

		const std::shared_ptr<const WavSoundImpl> wavSound;
		
		std::uint32_t curSmp = 0;
	
	public:
		Source(const std::shared_ptr<const WavSoundImpl>& sound) :
				wavSound(ASS(sound))
		{}

	private:
		bool fillSampleBuffer(utki::Buf<Frame<frame_type>> buf)noexcept override{
			ASSERT(this->wavSound->data.size() % audout::AudioFormat::numChannels(frame_type) == 0)
			ASSERT(this->curSmp % audout::AudioFormat::numChannels(frame_type) == 0)
			
			size_t framesToCopy = (this->wavSound->data.size() - this->curSmp) / audout::AudioFormat::numChannels(frame_type);
			utki::clampTop(framesToCopy, buf.size());

			ASSERT(framesToCopy <= buf.size())
			
			if(framesToCopy == 0){
				//TODO: check number of replays
				return true;
			}

			ASSERT(this->curSmp <= this->wavSound->data.size())			
			const TSampleType *startSmp = &this->wavSound->data[this->curSmp];
			
			this->curSmp += framesToCopy * audout::AudioFormat::numChannels(frame_type);
			
			auto dst = buf.begin();
			for(const TSampleType *src = startSmp; dst != buf.begin() + framesToCopy; ++dst){
				for(unsigned i = 0; i != audout::AudioFormat::numChannels(frame_type); ++i, ++src){
					dst->channel[i] = std::int32_t(*src);
				}
			}

			//fill the rest with zeroes
			for(; dst != buf.end(); ++dst){
				for(auto& c : dst->channel){
					c = 0;
				}
			}
			return false;
		}
	};//~class Source

private:
	std::shared_ptr<aumiks::ASource> createSource(std::uint32_t frequency = 0)const override{
		auto src = utki::makeShared<Source>(this->sharedFromThis(this));
		if(frequency == 0 || frequency == this->frequency()){
			return src;
		}
		
		auto resampler = utki::makeShared<Resampler<frame_type>>();
		
		resampler->input().connect(std::move(src));
		
		resampler->setScale(this->frequency(), frequency);
		
		return resampler;
	}
	
public:
	//NOTE: assume that data in d is little-endian
	WavSoundImpl(const utki::Buf<std::uint8_t> d, std::uint32_t frequency) :
			WavSound(audout::AudioFormat::numChannels(frame_type), frequency)
	{
		ASSERT(d.size() % (this->numChannels() * sizeof(TSampleType)) == 0)

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



}//~namespace



std::shared_ptr<WavSound> WavSound::load(const std::string& fileName){
	papki::FSFile fi(fileName);
	return WavSound::load(fi);
}



std::shared_ptr<WavSound> WavSound::load(papki::File& fi){
	papki::File::Guard fileGuard(fi, papki::File::E_Mode::READ);//make sure we close the file even in case of exception is thrown

	//Start reading Wav-file header
	{
		std::array<std::uint8_t, 4> riff;
		fi.read(utki::wrapBuf(riff));//Read 'RIFF' signature
		if(std::string(reinterpret_cast<char*>(&*riff.begin()), riff.size()) != "RIFF"){
			throw Exc("WavSound::LoadWAV(): 'RIFF' signature not found");
		}
	}

	fi.seekForward(4);//Skip "Wav-file size minus 7". We are not interested in this information

	{
		std::array<std::uint8_t, 4> wave;
		fi.read(utki::wrapBuf(wave));//Read 'WAVE' signature
		if(std::string(reinterpret_cast<char*>(&*wave.begin()), wave.size()) != "WAVE"){
			throw Exc("WavSound::LoadWAV(): 'WAVE' signature not found");
		}
	}

	{
		std::array<std::uint8_t, 4> fmt;
		fi.read(utki::wrapBuf(fmt));//Read 'fmt ' signature
		if(std::string(reinterpret_cast<char*>(&*fmt.begin()), fmt.size()) != "fmt "){
			throw Exc("WavSound::LoadWAV(): 'fmt ' signature not found");
		}
	}

	fi.seekForward(4);//Skip 4 bytes. Their purpose is unknown to me.

	unsigned chans;
	{
		std::array<std::uint8_t, 4> pcmBuf;
		fi.read(utki::wrapBuf(pcmBuf));
		std::uint32_t pcm = utki::deserialize32LE(&*pcmBuf.begin());
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
	std::uint32_t frequency;
	{
		std::array<std::uint8_t, 4> buf;
		fi.read(utki::wrapBuf(buf));
		frequency = utki::deserialize32LE(&*buf.begin());
	}

	fi.seekForward(4);//Playback speed (freq * PCMSampleSize). We don't need this info.

	std::uint32_t bitDepth;
	{
		std::array<std::uint8_t, 4> buf;
		fi.read(utki::wrapBuf(buf));
		bitDepth = utki::deserialize32LE(&*buf.begin());
		bitDepth >>= 16;//High word contains the sound bit depth
	}

	{
		std::array<std::uint8_t, 4> data;
		fi.read(utki::wrapBuf(data));//Read 'data' signature
		if(std::string(reinterpret_cast<char*>(&*data.begin()), data.size()) != "data"){
			throw Exc("WavSound::LoadWAV(): 'data' signature not found");
		}
	}

	std::uint32_t dataSize;
	{
		std::array<std::uint8_t, 4> buf;
		fi.read(utki::wrapBuf(buf));//read the size of the sound data
		dataSize = utki::deserialize32LE(&*buf.begin());
	}
	
	//read in the sound data
	std::vector<std::uint8_t> data(dataSize);
	{
		unsigned bytesRead = fi.read(utki::wrapBuf(data));//Load Sound data

		if(bytesRead != dataSize){
			throw Exc("WavSound::LoadWAV(): sound data size is incorrect");
		}
	}
	
	//Now we have Wav-file info
	std::shared_ptr<WavSound> ret;
	if(bitDepth == 8){
//		C_Ref<C_PCM_ParticularNonStreamedSound<s8> > r = new C_PCM_ParticularNonStreamedSound<s8>(chans, igagis::uint(frequency), dataSize);
//		ret = static_cast<C_PCM_NonStreamedSound*>(r.operator->());
//		bytesRead = fi.read(r->buf.Buf(), r->buf.SizeOfArray());//Load Sound data
//		//convert data to signed format
//		for(s8* ptr=r->buf.Buf(); ptr<(r->buf.Buf()+r->buf.SizeOfArray()); ++ptr)
//			*ptr=s8(int(*ptr)-0x80);
		//TODO: support it
		throw Exc("WavSound::LoadWAV(): unsupported bit depth (8 bit) wav file (TODO:)");
	}else if(bitDepth == 16){
		//set the format
		switch(chans){
			case 1://mono
				ret = utki::makeShared<WavSoundImpl<std::int16_t, audout::Frame_e::MONO>>(utki::wrapBuf(data), frequency);
				break;
			case 2://stereo
				ret = utki::makeShared<WavSoundImpl<std::int16_t, audout::Frame_e::STEREO>>(utki::wrapBuf(data), frequency);
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


