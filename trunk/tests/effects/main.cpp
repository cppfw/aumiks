#include "../../src/aumiks/WavSound.hpp"

#include <ting/FSFile.hpp>



class VolumeEffect : public aumiks::Effect{
public:
	volatile ting::u8 vol;
	
	//override
	virtual bool ApplyToSmpBuf44100Stereo16(ting::Buffer<ting::s32>& buf){
		for(ting::s32* i = buf.Begin(); i != buf.End(); ++i){
			*i = (*i) * this->vol / ting::u8(-1);
		}
		return true;
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
	
	ting::Ref<VolumeEffect> vol = VolumeEffect::New();
	vol->vol = 0xff;
	
	ch->AddEffect(vol);
	
	ch->Play(0);//infinite loop
	
	ting::s8 d = -1;
	ting::s8 step = 20;
	for(unsigned i = 0; i != 100; ++i){
//		TRACE(<< "Loop" << std::endl)
		ting::Thread::Sleep(100);
		
		if(vol->vol < step && d < 0){
			d = 1;
		}
		if(vol->vol > (0xff - step) && d > 0){
			d = -1;
		}
		vol->vol += d * step;
	}
	
	return 0;
}
