#include "../../src/aumiks/Lib.hpp"
#include "../../src/aumiks/Sound.hpp"
#include "../../src/aumiks/Channel.hpp"



class SineSound : public aumiks::Sound{
	
	class Channel : public aumiks::Channel{
		ting::Inited<float, 0> time;
		
		//override
		bool FillSmpBuf(utki::Buf<std::int32_t>& buf, unsigned freq, unsigned chans){
//			TRACE_ALWAYS(<< "filling smp buf, freq = " << freq << std::endl)
			
			if(this->time > 1){//play sound for 1 second
//				TRACE_ALWAYS(<< "returned true" << std::endl)
				return true;
			}
			
			for(std::int32_t* dst = buf.Begin(); dst != buf.End();){
				std::int32_t v = float(0x7fff) * ting::math::Sin<float>(this->time * ting::math::D2Pi<float>() * 440.0f);
				this->time += 1 / float(freq);
				for(unsigned i = 0; i != chans; ++i){
					ASSERT(buf.Overlaps(dst))
					*dst = v;
					++dst;
				}
			}
			
//			TRACE_ALWAYS(<< "time = " << this->time << std::endl)
//			TRACE(<< "this->smpBuf = " << buf << std::endl)
			
			return false;
		}
		
	public:
		inline static std::shared_ptr<Channel> New(){
			return std::shared_ptr<Channel>(new Channel());
		}
	};
	
public:
	
	//override
	virtual std::shared_ptr<aumiks::Channel> CreateChannel()const{
		return Channel::New();
	}
	
	inline static std::shared_ptr<SineSound> New(){
		return std::shared_ptr<SineSound>(new SineSound());
	}
};



void Play(){
	std::shared_ptr<SineSound> snd = SineSound::New();
	
	std::shared_ptr<aumiks::Channel> ch = snd->CreateChannel();
	ch->Play_ts();

	while(!ch->IsStopped_ts()){
		ting::mt::Thread::Sleep(333);
	}
}



int main(int argc, char *argv[]){
	
	{
		TRACE_ALWAYS(<< "Opening audio playback device: Mono 11025" << std::endl)
		aumiks::Lib aumiksLibrary(audout::AudioFormat(audout::AudioFormat::Frame::MONO, audout::AudioFormat::SamplingRate::HZ_11025), 100);

		Play();
		TRACE_ALWAYS(<< "finished playing" << std::endl)
	}
	
	{
		TRACE_ALWAYS(<< "Opening audio playback device: Stereo 11025" << std::endl)
		aumiks::Lib aumiksLibrary(audout::AudioFormat(audout::AudioFormat::Frame::STEREO, audout::AudioFormat::SamplingRate::HZ_11025), 100);

		Play();
	}
	
	{
		TRACE_ALWAYS(<< "Opening audio playback device: Mono 22050" << std::endl)
		aumiks::Lib aumiksLibrary(audout::AudioFormat(audout::AudioFormat::Frame::MONO, audout::AudioFormat::SamplingRate::HZ_22050), 100);

		Play();
	}
	
	{
		TRACE_ALWAYS(<< "Opening audio playback device: Stereo 22050" << std::endl)
		aumiks::Lib aumiksLibrary(audout::AudioFormat(audout::AudioFormat::Frame::STEREO, audout::AudioFormat::SamplingRate::HZ_22050), 100);

		Play();
	}
	
	{
		TRACE_ALWAYS(<< "Opening audio playback device: Mono 44100" << std::endl)
		aumiks::Lib aumiksLibrary(audout::AudioFormat(audout::AudioFormat::Frame::MONO, audout::AudioFormat::SamplingRate::HZ_44100), 100);

		Play();
	}
	
	{
		TRACE_ALWAYS(<< "Opening audio playback device: Stereo 44100" << std::endl)
		aumiks::Lib aumiksLibrary(audout::AudioFormat(audout::AudioFormat::Frame::STEREO, audout::AudioFormat::SamplingRate::HZ_44100), 100);

		Play();
	}
	
	return 0;
}
