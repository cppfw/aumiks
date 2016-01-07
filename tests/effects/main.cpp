#include "../../src/aumiks/Lib.hpp"
#include "../../src/aumiks/WavSound.hpp"

#include <papki/FSFile.hpp>
#include <utki/util.hpp>



class SineSound : public aumiks::Sound{
	
	class Channel : public aumiks::Channel{
		ting::Inited<float, 0> time;
		
		//override
		bool FillSmpBuf(utki::Buf<std::int32_t>& buf, unsigned freq, unsigned chans){
//			TRACE_ALWAYS(<< "filling smp buf, freq = " << freq << std::endl)
			
			if(this->time > 5){//play sound for 5 seconds
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



namespace TestNonTimeDomainEffect{

class VolumeEffect : public aumiks::Effect{
	ting::Inited<volatile std::uint8_t, std::uint8_t(-1)> vol;
public:
	inline void SetVolume(std::uint8_t vol){
		this->vol = vol;
	}
	
	//override
	virtual bool FillSmpBuf(utki::Buf<std::int32_t>& buf, unsigned freq, unsigned chans){
//		TRACE_ALWAYS(<< "effect FillSmpBuf(): enter" << std::endl)
		
		if(this->FillSmpBufFromNextByChain(buf, freq, chans)){
//			TRACE_ALWAYS(<< "effect FillSmpBuf(): true from next by chain" << std::endl)
			return true;
		}
		
		std::uint8_t vol = this->vol; //save volatile value
		if(vol == std::uint8_t(-1)){
			//do nothing
			return false;
		}
		
		for(std::int32_t* i = buf.Begin(); i != buf.End(); ++i){
			*i = (*i) * vol / std::uint8_t(-1);
		}
		
		return false;
	}
	
	static inline std::shared_ptr<VolumeEffect> New(){
		return std::shared_ptr<VolumeEffect>(new VolumeEffect());
	}
};

void Run(){
	TRACE_ALWAYS(<< "Opening audio playback device: Stereo 44100" << std::endl)
	aumiks::Lib aumiksLibrary(audout::AudioFormat(audout::AudioFormat::Frame::STEREO, audout::AudioFormat::SamplingRate::HZ_44100), 100);
	
	std::shared_ptr<aumiks::Sound> snd = SineSound::New();

	ASSERT(snd)
	
	std::shared_ptr<aumiks::Channel> ch = snd->CreateChannel();
//	TRACE_ALWAYS(<< "ch = " << static_cast<aumiks::SampleBufferFiller*>(ch.operator->()) << std::endl)
	
	std::shared_ptr<VolumeEffect> eff = VolumeEffect::New();
	
	ch->AddEffect_ts(eff);
	
	ch->Play_ts();
	
	ting::s8 d = -1;
	ting::s8 step = 20;
	std::uint8_t vol = 0xff;
	for(unsigned i = 0; i != 100; ++i){
//		TRACE(<< "Loop" << std::endl)
		ting::mt::Thread::Sleep(100);
		
		if(vol < step && d < 0){
			d = 1;
		}
		if(vol > (0xff - step) && d > 0){
			d = -1;
		}
		vol += d * step;
		eff->SetVolume(vol);
	}
}

}//~namespace



namespace TestResamplerEffect{

class ResampleEffect : public aumiks::Effect{
	
	//override
	virtual bool FillSmpBuf(utki::Buf<std::int32_t>& buf, unsigned freq, unsigned chans){
		if(this->FillSmpBufFromNextByChain(buf, freq, chans)){
//			TRACE_ALWAYS(<< "effect FillSmpBuf(): true from next by chain" << std::endl)
			return true;
		}
		
		//TODO:
		return false;
	}
	
public:
	static inline std::shared_ptr<ResampleEffect> New(){
		return std::shared_ptr<ResampleEffect>(
				new ResampleEffect()
			);
	}
	//TODO:
};

void Run(){
	TRACE_ALWAYS(<< "Opening audio playback device: Stereo 44100" << std::endl)
	aumiks::Lib aumiksLibrary(audout::AudioFormat(audout::AudioFormat::Frame::STEREO, audout::AudioFormat::SamplingRate::HZ_44100), 100);
	
	std::shared_ptr<aumiks::Sound> snd = SineSound::New();

	ASSERT(snd)
	
	std::shared_ptr<aumiks::Channel> ch = snd->CreateChannel();
//	TRACE_ALWAYS(<< "ch = " << static_cast<aumiks::SampleBufferFiller*>(ch.operator->()) << std::endl)
	
	std::shared_ptr<ResampleEffect> eff = ResampleEffect::New();
	
	ch->AddEffect_ts(eff);
	
	ch->Play_ts();
	
	while(!ch->IsStopped_ts()){
		ting::mt::Thread::Sleep(333);
	}
}

}//~namespace



int main(int argc, char *argv[]){
	
	TestResamplerEffect::Run();
	TestNonTimeDomainEffect::Run();
	
	return 0;
}
