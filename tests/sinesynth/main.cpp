#include <ting/math.hpp>

#include "../../src/aumiks/Lib.hpp"
#include "../../src/aumiks/Sound.hpp"
#include "../../src/aumiks/Channel.hpp"



class SineSound : public aumiks::Sound{
	
	class Channel : public aumiks::Channel{
		ting::Inited<float, 0> time;
		
		//override
		bool FillSmpBuf(ting::Buffer<ting::s32>& buf, unsigned freq, unsigned chans){
//			TRACE_ALWAYS(<< "filling smp buf, freq = " << freq << std::endl)
			
			if(this->time > 1){//play sound for 1 second
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



void Play(){
	ting::Ref<SineSound> snd = SineSound::New();
	
	ting::Ref<aumiks::Channel> ch = snd->CreateChannel();
	ch->Play_ts();

	while(!ch->IsStopped_ts()){
		ting::mt::Thread::Sleep(333);
	}
}



int main(int argc, char *argv[]){
	
	{
		TRACE_ALWAYS(<< "Opening audio playback device: Mono 11025" << std::endl)
		aumiks::Lib aumiksLibrary(11025, 1, 100);

		Play();
		TRACE_ALWAYS(<< "finished playing" << std::endl)
	}
	
	{
		TRACE_ALWAYS(<< "Opening audio playback device: Stereo 11025" << std::endl)
		aumiks::Lib aumiksLibrary(11025, 2, 100);

		Play();
	}
	
	{
		TRACE_ALWAYS(<< "Opening audio playback device: Mono 22050" << std::endl)
		aumiks::Lib aumiksLibrary(22050, 1, 100);

		Play();
	}
	
	{
		TRACE_ALWAYS(<< "Opening audio playback device: Stereo 22050" << std::endl)
		aumiks::Lib aumiksLibrary(22050, 2, 100);

		Play();
	}
	
	{
		TRACE_ALWAYS(<< "Opening audio playback device: Mono 44100" << std::endl)
		aumiks::Lib aumiksLibrary(44100, 1, 100);

		Play();
	}
	
	{
		TRACE_ALWAYS(<< "Opening audio playback device: Stereo 44100" << std::endl)
		aumiks::Lib aumiksLibrary(44100, 2, 100);

		Play();
	}
	
	return 0;
}
