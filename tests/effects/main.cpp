#include "../../src/aumiks/Lib.hpp"
#include "../../src/aumiks/WavSound.hpp"

#include <ting/fs/FSFile.hpp>
#include <ting/util.hpp>



class SineSound : public aumiks::Sound{
	
	class Channel : public aumiks::Channel{
		ting::Inited<float, 0> time;
		
		//override
		bool FillSmpBuf(ting::Buffer<ting::s32>& buf, unsigned freq, unsigned chans){
//			TRACE_ALWAYS(<< "filling smp buf, freq = " << freq << std::endl)
			
			if(this->time > 5){//play sound for 5 seconds
//				TRACE_ALWAYS(<< "returned true" << std::endl)
				return true;
			}
			
			for(ting::s32* dst = buf.Begin(); dst != buf.End();){
				ting::s32 v = float(0x7fff) * ting::math::Sin<float>(this->time * ting::math::D2Pi<float>() * 440.0f);
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
		inline static ting::Ref<Channel> New(){
			return ting::Ref<Channel>(new Channel());
		}
	};
	
public:
	
	//override
	virtual ting::Ref<aumiks::Channel> CreateChannel()const{
		return Channel::New();
	}
	
	inline static ting::Ref<SineSound> New(){
		return ting::Ref<SineSound>(new SineSound());
	}
};



namespace TestNonTimeDomainEffect{

class VolumeEffect : public aumiks::Effect{
	ting::Inited<volatile ting::u8, ting::u8(-1)> vol;
public:
	inline void SetVolume(ting::u8 vol){
		this->vol = vol;
	}
	
	//override
	virtual bool FillSmpBuf(ting::Buffer<ting::s32>& buf, unsigned freq, unsigned chans){
//		TRACE_ALWAYS(<< "effect FillSmpBuf(): enter" << std::endl)
		
		if(this->FillSmpBufFromNextByChain(buf, freq, chans)){
//			TRACE_ALWAYS(<< "effect FillSmpBuf(): true from next by chain" << std::endl)
			return true;
		}
		
		ting::u8 vol = this->vol; //save volatile value
		if(vol == ting::u8(-1)){
			//do nothing
			return false;
		}
		
		for(ting::s32* i = buf.Begin(); i != buf.End(); ++i){
			*i = (*i) * vol / ting::u8(-1);
		}
		
		return false;
	}
	
	static inline ting::Ref<VolumeEffect> New(){
		return ting::Ref<VolumeEffect>(new VolumeEffect());
	}
};

void Run(){
	TRACE_ALWAYS(<< "Opening audio playback device: Stereo 44100" << std::endl)
	aumiks::Lib aumiksLibrary(audout::AudioFormat(audout::AudioFormat::Frame::STEREO, audout::AudioFormat::SamplingRate::HZ_44100), 100);
	
	ting::Ref<aumiks::Sound> snd = SineSound::New();

	ASSERT(snd)
	
	ting::Ref<aumiks::Channel> ch = snd->CreateChannel();
//	TRACE_ALWAYS(<< "ch = " << static_cast<aumiks::SampleBufferFiller*>(ch.operator->()) << std::endl)
	
	ting::Ref<VolumeEffect> eff = VolumeEffect::New();
	
	ch->AddEffect_ts(eff);
	
	ch->Play_ts();
	
	ting::s8 d = -1;
	ting::s8 step = 20;
	ting::u8 vol = 0xff;
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
	virtual bool FillSmpBuf(ting::Buffer<ting::s32>& buf, unsigned freq, unsigned chans){
		if(this->FillSmpBufFromNextByChain(buf, freq, chans)){
//			TRACE_ALWAYS(<< "effect FillSmpBuf(): true from next by chain" << std::endl)
			return true;
		}
		
		//TODO:
		return false;
	}
	
public:
	static inline ting::Ref<ResampleEffect> New(){
		return ting::Ref<ResampleEffect>(
				new ResampleEffect()
			);
	}
	//TODO:
};

void Run(){
	TRACE_ALWAYS(<< "Opening audio playback device: Stereo 44100" << std::endl)
	aumiks::Lib aumiksLibrary(audout::AudioFormat(audout::AudioFormat::Frame::STEREO, audout::AudioFormat::SamplingRate::HZ_44100), 100);
	
	ting::Ref<aumiks::Sound> snd = SineSound::New();

	ASSERT(snd)
	
	ting::Ref<aumiks::Channel> ch = snd->CreateChannel();
//	TRACE_ALWAYS(<< "ch = " << static_cast<aumiks::SampleBufferFiller*>(ch.operator->()) << std::endl)
	
	ting::Ref<ResampleEffect> eff = ResampleEffect::New();
	
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
