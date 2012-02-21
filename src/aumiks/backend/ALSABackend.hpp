/* The MIT License:

Copyright (c) 2009-2011 Ivan Gagis

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE. */

/*
 * aumiks 0.1
 *
 * File:   ALSA.hpp
 * Author: ivan
 *
 * Created on March 9, 2009, 3:22 PM
 */

#pragma once

// Use the newer ALSA API
#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>

#include "WriteBasedBackend.hpp"

#include "../Exc.hpp"

namespace{

class ALSABackend : public WriteBasedBackend{
	struct Device{
		snd_pcm_t *handle;
		
		Device(){
			//Open PCM device for playback.
			if(snd_pcm_open(&this->handle, "default" /*"hw:0,0"*/, SND_PCM_STREAM_PLAYBACK, 0) < 0){
//				TRACE(<< "ALSA: unable to open pcm device" << std::endl)
				throw aumiks::Exc("ALSA: unable to open pcm device");
			}
		}
		
		~Device(){
			snd_pcm_close(this->handle);
		}
	} device;

	unsigned bytesPerFrame;
	
public:
	ALSABackend(unsigned bufferSizeFrames, aumiks::E_Format format) :
			WriteBasedBackend(bufferSizeFrames * aumiks::BytesPerFrame(format)),
			bytesPerFrame(aumiks::BytesPerFrame(format))
	{
//		TRACE(<< "setting HW params" << std::endl)

		this->SetHWParams(bufferSizeFrames, format);

//		TRACE(<< "setting SW params" << std::endl)

		this->SetSwParams(bufferSizeFrames);//must be called after this->SetHWParams()!!!

		if(snd_pcm_prepare(this->device.handle) < 0){
//			TRACE(<< "cannot prepare audio interface for use" << std::endl)
			throw aumiks::Exc("cannot set parameters");
		}
		
		this->Start();//start thread
	}

	virtual ~ALSABackend()throw(){
		this->StopThread();
	}
	
	inline static ting::Ptr<ALSABackend> New(unsigned bufferSizeFrames, aumiks::E_Format format){
		return ting::Ptr<ALSABackend>(
				new ALSABackend(bufferSizeFrames, format)
			);
	}

	int RecoverALSAFromXrun(int err){
		TRACE(<< "stream recovery" << std::endl)
		if(err == -EPIPE){// under-run
			err = snd_pcm_prepare(this->device.handle);
			if (err < 0){
				TRACE(
						<< "Can't recovery from underrun, prepare failed, error code ="
						<< snd_strerror(err) << std::endl
					)
			}
			return 0;
		}else if(err == -ESTRPIPE){
			while((err = snd_pcm_resume(this->device.handle)) == -EAGAIN)
				ting::Thread::Sleep(100);// wait until the suspend flag is released
			if(err < 0){
				err = snd_pcm_prepare(this->device.handle);
				if (err < 0){
					TRACE(
							<< "Can't recovery from suspend, prepare failed, error code ="
							<< snd_strerror(err) << std::endl
						)
				}
			}
			return 0;
		}
		return err;
	}

	//override
	void Write(const ting::Buffer<ting::u8>& buf){
		ASSERT(buf.Size() % this->bytesPerFrame == 0)
		
		unsigned bufferSizeFrames = buf.Size() / this->bytesPerFrame;
		
		unsigned numFramesWritten = 0;
		while(numFramesWritten < bufferSizeFrames){
			//write interleaved samples
			int ret = snd_pcm_writei(
					this->device.handle,
					reinterpret_cast<const void*>(&buf[numFramesWritten * this->bytesPerFrame]),
					bufferSizeFrames - numFramesWritten
				);
			if(ret < 0){
				if(ret == -EAGAIN)
					continue;

				int err = this->RecoverALSAFromXrun(ret);
				if(err < 0){
//					LOG(<< "write to audio interface failed, err = " << snd_strerror(err) << std::endl)
					throw aumiks::Exc("write to audio interface failed");
				}
			}
			numFramesWritten += ret;
		}
	}

	void SetHWParams(unsigned bufferSizeFrames, aumiks::E_Format format){
		struct HwParams{
			snd_pcm_hw_params_t* params;
			
			HwParams(){
				if(snd_pcm_hw_params_malloc(&this->params) < 0){
					TRACE(<< "cannot allocate hardware parameter structure" << std::endl)
					throw aumiks::Exc("cannot allocate hardware parameter structure");
				}
			}
			
			~HwParams(){
				snd_pcm_hw_params_free(this->params);
			}
		} hw;

		if(snd_pcm_hw_params_any(this->device.handle, hw.params) < 0){
			TRACE(<< "cannot initialize hardware parameter structure" << std::endl)
			throw aumiks::Exc("cannot initialize hardware parameter structure");
		}

		if(snd_pcm_hw_params_set_access(this->device.handle, hw.params, SND_PCM_ACCESS_RW_INTERLEAVED) < 0){
			TRACE(<< "cannot set access type" << std::endl)
			throw aumiks::Exc("cannot set access type");
		}

		if(snd_pcm_hw_params_set_format(this->device.handle, hw.params, SND_PCM_FORMAT_S16_LE) < 0){
			TRACE(<< "cannot set sample format" << std::endl)
			throw aumiks::Exc("cannot set sample format");
		}

		{
			unsigned val = aumiks::SamplingRate(format);
			if(snd_pcm_hw_params_set_rate_near(this->device.handle, hw.params, &val, 0) < 0){
				TRACE(<< "cannot set sample rate" << std::endl)
				throw aumiks::Exc("cannot set sample rate");
			}
		}

		if(snd_pcm_hw_params_set_channels(this->device.handle, hw.params, aumiks::SamplesPerFrame(format)) < 0){
			TRACE(<< "cannot set channel count" << std::endl)
			throw aumiks::Exc("cannot set channel count");
		}

		//Set period size
		{
			snd_pcm_uframes_t frames = snd_pcm_uframes_t(bufferSizeFrames);
			int dir = 0;
			if(snd_pcm_hw_params_set_period_size_near(
					this->device.handle,
					hw.params,
					&frames,
					&dir
				) < 0
			)
			{
				TRACE(<< "could not set period size" << std::endl)
				throw aumiks::Exc("could not set period size");
			}

			TRACE(<< "buffer size in samples = " << this->BufferSizeInSamples() << std::endl)
		}

		// Set number of periods. Periods used to be called fragments.
		{
			unsigned int numPeriods = 2;
			int err = snd_pcm_hw_params_set_periods_near(this->device.handle, hw.params, &numPeriods, NULL);
			if(err < 0){
				TRACE(<< "could not set number of periods, err = " << err << std::endl)
				throw aumiks::Exc("could not set number of periods");
			}
			TRACE(<< "numPeriods = " << numPeriods << std::endl)
		}


		//set hw params
		if(snd_pcm_hw_params(this->device.handle, hw.params) < 0){
			TRACE(<< "cannot set parameters" << std::endl)
			throw aumiks::Exc("cannot set parameters");
		}
	}

	

	void SetSwParams(unsigned bufferSizeFrames){
		struct SwParams{
			snd_pcm_sw_params_t *params;
			SwParams(){
				if(snd_pcm_sw_params_malloc(&this->params) < 0){
					TRACE(<< "cannot allocate software parameters structure" << std::endl)
					throw aumiks::Exc("cannot allocate software parameters structure");
				}
			}
			~SwParams(){
				snd_pcm_sw_params_free(this->params);
			}
		} sw;

		if(snd_pcm_sw_params_current(this->device.handle, sw.params) < 0){
			TRACE(<< "cannot initialize software parameters structure" << std::endl)
			throw aumiks::Exc("cannot initialize software parameters structure");
		}

		//tell ALSA to wake us up whenever 'buffer size' frames of playback data can be delivered
		if(snd_pcm_sw_params_set_avail_min(this->device.handle, sw.params, bufferSizeFrames) < 0){
			TRACE(<< "cannot set minimum available count" << std::endl)
			throw aumiks::Exc("cannot set minimum available count");
		}

		//tell ALSA to start playing on first data write
		if(snd_pcm_sw_params_set_start_threshold(this->device.handle, sw.params, 0) < 0){
			TRACE(<< "cannot set start mode" << std::endl)
			throw aumiks::Exc("cannot set start mode");
		}

		if(snd_pcm_sw_params(this->device.handle, sw.params) < 0){
			TRACE(<< "cannot set software parameters" << std::endl)
			throw aumiks::Exc("cannot set software parameters");
		}
	}
	
};

}//~namespace
