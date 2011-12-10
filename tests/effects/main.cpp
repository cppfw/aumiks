#include "../../src/aumiks/WavSound.hpp"

#include <ting/FSFile.hpp>
#include <ting/utils.hpp>



class VolumeEffect : public aumiks::Effect{
	ting::Inited<volatile ting::u8, ting::u8(-1)> vol;
public:
	inline void SetVolume(ting::u8 vol){
		this->vol = vol;
	}
	
	//override
	virtual aumiks::Effect::E_Result ApplyToSmpBuf44100Stereo16(ting::Buffer<ting::s32>& buf, bool soundStopped){
		ting::u8 vol = this->vol; //save volatile value
		if(vol == ting::u8(-1)){
			//do nothing
			return aumiks::Effect::NORMAL;
		}
		
		for(ting::s32* i = buf.Begin(); i != buf.End(); ++i){
			*i = (*i) * vol / ting::u8(-1);
		}
		
		return aumiks::Effect::NORMAL;
	}
	
	static inline ting::Ref<VolumeEffect> New(){
		return ting::Ref<VolumeEffect>(new VolumeEffect());
	}
};


int main(int argc, char *argv[]){
	TRACE_ALWAYS(<< "Opening audio playback device: Stereo 44100" << std::endl)
	aumiks::Lib aumiksLibrary(100, aumiks::STEREO_16_44100);
	
	ting::Ref<aumiks::WavSound> snd = aumiks::WavSound::LoadWAV("../samples/sine44100mono16.wav");

	ASSERT(snd)
	
	ting::Ref<aumiks::WavSound::Channel> ch = snd->CreateWavChannel();
	
	ting::Ref<VolumeEffect> eff = VolumeEffect::New();
	
	ch->AddEffect_ts(eff);
	
	ch->Play(0);//infinite loop
	
	ting::s8 d = -1;
	ting::s8 step = 20;
	ting::u8 vol = 0xff;
	for(unsigned i = 0; i != 100; ++i){
//		TRACE(<< "Loop" << std::endl)
		ting::Thread::Sleep(100);
		
		if(vol < step && d < 0){
			d = 1;
		}
		if(vol > (0xff - step) && d > 0){
			d = -1;
		}
		vol += d * step;
		eff->SetVolume(vol);
	}
	
	return 0;
}
