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
			
			if(this->time > 2){
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




int main(int argc, char *argv[]){
	ting::Ref<SineSound> snd = SineSound::New();
	
	{
		TRACE_ALWAYS(<< "Opening audio playback device: Mono 11025" << std::endl)
		aumiks::Lib aumiksLibrary(11025, 1, 100);

		ting::Ref<aumiks::Channel> ch = snd->CreateChannel();
		ch->Play_ts();
		
		ting::Thread::Sleep(2000);
		TRACE_ALWAYS(<< "finished playing" << std::endl)
	}
	
//	{
//		TRACE_ALWAYS(<< "Opening audio playback device: Stereo 11025" << std::endl)
//		aumiks::Lib aumiksLibrary(100, aumiks::STEREO_16_11025);
//
//		Play("../samples/sample11025mono16.wav");
//		Play("../samples/sample11025stereo16.wav");
//		Play("../samples/sample22050mono16.wav");
//		Play("../samples/sample22050stereo16.wav");
//		Play("../samples/sample44100mono16.wav");
//		Play("../samples/sample44100stereo16.wav");
//	}
//	
//	{
//		TRACE_ALWAYS(<< "Opening audio playback device: Mono 22050" << std::endl)
//		aumiks::Lib aumiksLibrary(100, aumiks::MONO_16_22050);
//
//		Play("../samples/sample11025mono16.wav");
//		Play("../samples/sample11025stereo16.wav");
//		Play("../samples/sample22050mono16.wav");
//		Play("../samples/sample22050stereo16.wav");
//		Play("../samples/sample44100mono16.wav");
//		Play("../samples/sample44100stereo16.wav");
//	}
//	
//	{
//		TRACE_ALWAYS(<< "Opening audio playback device: Stereo 22050" << std::endl)
//		aumiks::Lib aumiksLibrary(100, aumiks::STEREO_16_22050);
//
//		Play("../samples/sample11025mono16.wav");
//		Play("../samples/sample11025stereo16.wav");
//		Play("../samples/sample22050mono16.wav");
//		Play("../samples/sample22050stereo16.wav");
//		Play("../samples/sample44100mono16.wav");
//		Play("../samples/sample44100stereo16.wav");
//	}
//	
//	{
//		TRACE_ALWAYS(<< "Opening audio playback device: Mono 44100" << std::endl)
//		aumiks::Lib aumiksLibrary(100, aumiks::MONO_16_44100);
//
//		Play("../samples/sample11025mono16.wav");
//		Play("../samples/sample11025stereo16.wav");
//		Play("../samples/sample22050mono16.wav");
//		Play("../samples/sample22050stereo16.wav");
//		Play("../samples/sample44100mono16.wav");
//		Play("../samples/sample44100stereo16.wav");
//	}
//	
//	{
//		TRACE_ALWAYS(<< "Opening audio playback device: Stereo 44100" << std::endl)
//		aumiks::Lib aumiksLibrary(100, aumiks::STEREO_16_44100);
//
//		Play("../samples/sample11025mono16.wav");
//		Play("../samples/sample11025stereo16.wav");
//		Play("../samples/sample22050mono16.wav");
//		Play("../samples/sample22050stereo16.wav");
//		Play("../samples/sample44100mono16.wav");
//		Play("../samples/sample44100stereo16.wav");
//	}
	
	return 0;
}
