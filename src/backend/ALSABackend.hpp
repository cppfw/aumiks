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

/* Use the newer ALSA API */
#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>

#include <ting/Thread.hpp>

#include "AudioBackend.hpp"

#include "../Exc.hpp"

namespace{

class ALSABackend : public AudioBackend{
	snd_pcm_t *handle;

public:
	ALSABackend(unsigned requestedBufferSizeInFrames){
		//TODO:make wrapper for this->handle

		TRACE(<< "opening device" << std::endl)

		//Open PCM device for playback.
		if(snd_pcm_open(&this->handle, "default" /*"hw:0,0"*/, SND_PCM_STREAM_PLAYBACK, 0) < 0){
			TRACE(<< "ALSA: unable to open pcm device" << std::endl)
			throw aumiks::Exc("ALSA: unable to open pcm device");
		}

		TRACE(<< "setting HW params" << std::endl)

		this->SetHWParams(requestedBufferSizeInFrames);

		TRACE(<< "setting SW params" << std::endl)

		this->SetSwParams();//must be called after this->SetHWParams()!!!

		if(snd_pcm_prepare(this->handle) < 0){
			TRACE(<< "cannot prepare audio interface for use" << std::endl)
			throw aumiks::Exc("cannot set parameters");
		}
	}

	~ALSABackend(){
		snd_pcm_close(this->handle);
	}

	int RecoverALSAFromXrun(int err){
		TRACE(<< "stream recovery" << std::endl)
		if(err == -EPIPE){// under-run
			err = snd_pcm_prepare(this->handle);
			if (err < 0){
				TRACE(
						<< "Can't recovery from underrun, prepare failed, error code ="
						<< snd_strerror(err) << std::endl
					)
			}
			return 0;
		}else if(err == -ESTRPIPE){
			while((err = snd_pcm_resume(this->handle)) == -EAGAIN)
				ting::Thread::Sleep(100);// wait until the suspend flag is released
			if(err < 0){
				err = snd_pcm_prepare(this->handle);
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
		ASSERT(buf.Size() == this->BufferSizeInBytes())

		// wait till the interface is ready for data, or 1 second has elapsed.
//		{
//			int ret = snd_pcm_wait(s->handle, 500);
//			if(ret == 0){ //timeout occured
//				TRACE(<< "snd_pcm_wait() timeout occured" << std::endl)
//				LOG(<< "snd_pcm_wait() timeout occured" << std::endl)
//				continue;
//			}
//			if(ret < 0){
//				TRACE(<< "snd_pcm_wait() failed" << std::endl)
//				LOG(<< "snd_pcm_wait() failed" << std::endl)
//				ret = RecoverALSAFromXrun(s->handle, ret);
//				if(ret < 0){
//					TRACE(<< "RecoverALSAFromXrun() failed, error code = " << snd_strerror(ret) << std::endl)
//					LOG(<< "RecoverALSAFromXrun() failed" << snd_strerror(ret) << std::endl)
//					throw aumiks::Exc("snd_pcm_wait() failed and could not recover");
//				}
//			}
//		}
//
//		//check how much space is available for playback data
//		snd_pcm_sframes_t numFrames = 0;
//		do{
//			if((numFrames = snd_pcm_avail_update(s->handle)) < 0){
//				TRACE(<< "snd_pcm_avail_update() failed" << std::endl)
//				LOG(<< "snd_pcmsnd_pcm_avail_update_wait() failed" << std::endl)
//				int ret = RecoverALSAFromXrun(s->handle, int(numFrames));
//				if(ret < 0){
//					TRACE(<< "RecoverALSAFromXrun() failed, error code = " << snd_strerror(ret) << std::endl)
//					LOG(<< "RecoverALSAFromXrun() failed" << snd_strerror(ret) << std::endl)
//					throw aumiks::Exc("snd_pcm_avail_update() failed and could not recover");
//				}
//			}
//		}while(unsigned(numFrames) < s->BufferSizeInFrames());

//		TRACE(<< "writing data" << std::endl)

		unsigned numFramesWritten = 0;
		while(numFramesWritten < this->BufferSizeInFrames()){
			//write interleived samples
			int ret = snd_pcm_writei(
					this->handle,
					reinterpret_cast<const void*>(&buf[numFramesWritten * this->FrameSizeInBytes()]),
					this->BufferSizeInFrames() - numFramesWritten
				);
			if(ret < 0){
				if(ret == -EAGAIN)
					continue;

				int err = this->RecoverALSAFromXrun(ret);
				if(err < 0){
					LOG(<< "write to audio interface failed, err = " << snd_strerror(err) << std::endl)
					throw aumiks::Exc("write to audio interface failed");
				}
			}
			numFramesWritten += ret;
		}
	}

	void SetHWParams(unsigned requestedBufferSizeInFrames){
		struct CHwParamsWrapper{
			snd_pcm_hw_params_t *params;
			CHwParamsWrapper() : params(0) {};
			~CHwParamsWrapper(){
				if(this->params)
					snd_pcm_hw_params_free(this->params);
			}
		} hw;

		if(snd_pcm_hw_params_malloc(&hw.params) < 0){
			TRACE(<< "cannot allocate hardware parameter structure" << std::endl)
			throw aumiks::Exc("cannot allocate hardware parameter structure");
		}

		if(snd_pcm_hw_params_any(this->handle, hw.params) < 0){
			TRACE(<< "cannot initialize hardware parameter structure" << std::endl)
			throw aumiks::Exc("cannot initialize hardware parameter structure");
		}

		if(snd_pcm_hw_params_set_access(this->handle, hw.params, SND_PCM_ACCESS_RW_INTERLEAVED) < 0){
			TRACE(<< "cannot set access type" << std::endl)
			throw aumiks::Exc("cannot set access type");
		}

		if(snd_pcm_hw_params_set_format(this->handle, hw.params, SND_PCM_FORMAT_S16_LE) < 0){
			TRACE(<< "cannot set sample format" << std::endl)
			throw aumiks::Exc("cannot set sample format");
		}
		this->sampleSizeInBytes = 2;

		{
			unsigned val = 44100;
			if(snd_pcm_hw_params_set_rate_near(this->handle, hw.params, &val, 0) < 0){
				TRACE(<< "cannot set sample rate" << std::endl)
				throw aumiks::Exc("cannot set sample rate");
			}
		}

		if(snd_pcm_hw_params_set_channels(this->handle, hw.params, 2) < 0){
			TRACE(<< "cannot set channel count" << std::endl)
			throw aumiks::Exc("cannot set channel count");
		}
		this->numChannels = 2;

		//Set period size
		{
			snd_pcm_uframes_t frames = snd_pcm_uframes_t(requestedBufferSizeInFrames);
			int dir = 0;
			if(snd_pcm_hw_params_set_period_size_near(
					this->handle,
					hw.params,
					&frames,
					&dir
				) < 0
			)
			{
				TRACE(<< "could not set period size" << std::endl)
				throw aumiks::Exc("could not set period size");
			}
			this->bufSizeInFrames = frames;//save actual buffer size

			TRACE(<< "buffer size in samples = " << this->BufferSizeInSamples() << std::endl)
		}

		// Set number of periods. Periods used to be called fragments.
		{
			unsigned int numPeriods = 2;
			int err = snd_pcm_hw_params_set_periods_near(this->handle, hw.params, &numPeriods, NULL);
			if(err < 0){
				TRACE(<< "could not set number of periods, err = " << err << std::endl)
				throw aumiks::Exc("could not set number of periods");
			}
			TRACE(<< "numPeriods = " << numPeriods << std::endl)
		}


		//set hw params
		if(snd_pcm_hw_params(this->handle, hw.params) < 0){
			TRACE(<< "cannot set parameters" << std::endl)
			throw aumiks::Exc("cannot set parameters");
		}
	}

	

	void SetSwParams(){
		struct CSwParamsWrapper{
			snd_pcm_sw_params_t *params;
			CSwParamsWrapper() : params(0) {};
			~CSwParamsWrapper(){
				if(this->params)
					snd_pcm_sw_params_free(this->params);
			}
		} sw;

		if(snd_pcm_sw_params_malloc(&sw.params) < 0){
			TRACE(<< "cannot allocate software parameters structure" << std::endl)
			throw aumiks::Exc("cannot allocate software parameters structure");
		}

		if(snd_pcm_sw_params_current(this->handle, sw.params) < 0){
			TRACE(<< "cannot initialize software parameters structure" << std::endl)
			throw aumiks::Exc("cannot initialize software parameters structure");
		}

		//tell ALSA to wake us up whenever 'buffer size' frames of playback data can be delivered
		if(snd_pcm_sw_params_set_avail_min(this->handle, sw.params, this->BufferSizeInFrames()) < 0){
			TRACE(<< "cannot set minimum available count" << std::endl)
			throw aumiks::Exc("cannot set minimum available count");
		}

		//tell ALSA to start playing on first data write
		if(snd_pcm_sw_params_set_start_threshold(this->handle, sw.params, 0) < 0){
			TRACE(<< "cannot set start mode" << std::endl)
			throw aumiks::Exc("cannot set start mode");
		}

		if(snd_pcm_sw_params(this->handle, sw.params) < 0){
			TRACE(<< "cannot set software parameters" << std::endl)
			throw aumiks::Exc("cannot set software parameters");
		}
	}
	
};

}//~namespace
