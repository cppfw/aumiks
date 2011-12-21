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

// Homepage: http://aumiks.googlecode.com

/**
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once

#include <ting/Array.hpp>

#include <SLES/OpenSLES.h>
//#include <SLES/OpenSLES_Android.h> //TODO: remove this



namespace{

class OpenSLESBackend : public aumiks::AudioBackend{
	
	struct Engine{
		SLObjectItf object; //object
		SLEngineItf engine; //Engine interface
		
		Engine(){
			//create engine object
			if(slCreateEngine(&this->object, 0, NULL, 0, NULL, NULL) != SL_RESULT_SUCCESS){
				throw aumiks::Exc("OpenSLES: Creating engine object failed");
			}
			
			//realize the engine
			if((*this->object)->Realize(this->object, SL_BOOLEAN_FALSE) != SL_RESULT_SUCCESS){
				this->Destroy();
				throw aumiks::Exc("OpenSLES: Realizing engine object failed");
			}
			
			//get the engine interface, which is needed in order to create other objects
			if((*this->object)->GetInterface(this->object, SL_IID_ENGINE, &this->engine) != SL_RESULT_SUCCESS){
				this->Destroy();
				throw aumiks::Exc("OpenSLES: Obtaining Engine interface failed");
			}
		}
		
		~Engine(){
			this->Destroy();
		}
		
		void Destroy(){
			(*this->object)->Destroy(this->object);
		}
		
	private:
		Engine(const Engine&);
		Engine& operator=(const Engine&);
	} engine;
	
	
	
	struct OutputMix{
		SLObjectItf object;
		
		OutputMix(Engine& engine){
			if((*engine.engine)->CreateOutputMix(engine.engine, &this->object, 0, NULL, NULL) != SL_RESULT_SUCCESS){
				throw aumiks::Exc("OpenSLES: Creating output mix object failed");
			}
			
			// realize the output mix
			if((*this->object)->Realize(this->object, SL_BOOLEAN_FALSE) != SL_RESULT_SUCCESS){
				this->Destroy();
				throw aumiks::Exc("OpenSLES: Realizing output mix object failed");
			}
		}
		
		~OutputMix(){
			this->Destroy();
		}
		
		void Destroy(){
			(*this->object)->Destroy(this->object);
		}
		
	private:
		OutputMix(const OutputMix&);
		OutputMix& operator=(const OutputMix&);
	} outputMix;
	
	
	
	struct Player{
		SLObjectItf object;
		SLPlayItf play;
		SLBufferQueueItf bufferQueue;
		
		ting::StaticBuffer<ting::Array<ting::u8>, 2> bufs;
	
	
		//this callback handler is called every time a buffer finishes playing
		static void Callback(
				SLBufferQueueItf queueItf,
				SLuint32 eventFlags,
				const void *buffer,
				SLuint32 bufferSize,
				SLuint32 dataUsed,
				void *context
			)
		{
			if(eventFlags & SL_BUFFERQUEUEEVENT_STOPPED != 0){ //player was stopped
				return;
			}
			
			ASSERT(context)
			Player* player = static_cast<Player*>(context);
			
			ASSERT(buffer == player->bufs[0].Begin())
			ASSERT(bufferSize == player->bufs[0].Size())
			ASSERT(dataUsed <= bufferSize)
			
			ASSERT(player->bufs.Size() == 2)
			std::swap(player->bufs[0], player->bufs[1]); //swap buffers, the 0th one is the buffer which is currently playing
			
			SLresult res = (*queueItf)->Enqueue(queueItf, player->bufs[0].Begin(), player->bufs[0].Size());
			ASSERT(res == SL_RESULT_SUCCESS)
			
			//fill the second buffer to be enqueued next time the callback is called
			this->FillPlayBuf_ts(player->bufs[1]);
		}
		
		
		
		Player(Engine& engine, OutputMix& outputMix, unsigned bufferSizeFrames, aumiks::E_Format format){
			//Allocate play buffers of required size
			{
				size_t bufSize = bufferSizeFrames * aumiks::BytesPerFrame(format);
				for(ting::Array* i = this->bufs.Begin(); i != this->bufs.End(); ++i){
					i->Init(bufSize);
				}
				ASSERT(this->bufs.Size() == 2)
				//nitialize the second buffer with 0's, since playing will start from the second buffer
				memset(this->bufs[1].Begin(), 0, this->bufs[1].Size());
			}
			
			//========================
			// configure audio source
			//TODO: remove this
			//SLDataLocator_AndroidSimpleBufferQueue bufferQueue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2}; //2 buffers in queue
			SLDataLocator_BufferQueue bufferQueue = {SL_DATALOCATOR_BUFFERQUEUE, 2}; //2 buffers in queue
			
			unsigned numChannels = aumiks::SamplesPerFrame(format);
			SLuint32 channelMask;
			switch(numChannels){
				case 1:
					channelMask = SL_SPEAKER_FRONT_CENTER;
					break;
				case 2:
					channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
					break;
				default:
					ASSERT(false)
					break;
			}
			SLDataFormat_PCM audioFormat = {
				SL_DATAFORMAT_PCM,
				numChannels, //number of channels
				aumiks::SamplingRate(format) * 1000, //milliHertz
				16, //bits per sample, if 16bits then sample is signed 16bit integer
				16, //container size for sample, it can be bigger than sample itself. E.g. 32bit container for 16bits sample
				channelMask, //which channels map to which speakers
				SL_BYTEORDER_LITTLEENDIAN //we want little endian byte order
			};
			
			SLDataSource audioSourceStruct = {&bufferQueue, &audioFormat};

			//======================
			// configure audio sink
			SLDataLocator_OutputMix outputMixStruct = {SL_DATALOCATOR_OUTPUTMIX, outputMix.object};
			SLDataSink audioSinkStruct = {&outputMixStruct, NULL};

			//=====================
			//=====================
			// create audio player
			const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
			const SLboolean req[1] = {SL_BOOLEAN_TRUE};
			if((*engine.engine)->CreateAudioPlayer(
					engine.engine,
					&this->object,
					&audioSourceStruct,
					&audioSinkStruct,
					1,
					ids,
					req
				) != SL_RESULT_SUCCESS)
			{
				throw aumiks::Exc("OpenSLES: Creating player object failed");
			}
			
			//realize the player
			if((*this->object)->Realize(this->object, SL_BOOLEAN_FALSE) != SL_RESULT_SUCCESS){
				this->Destroy();
				throw aumiks::Exc("OpenSLES: Realizing player object failed");
			}

			//get the play interface
			if((*this->object)->GetInterface(this->object, SL_IID_PLAY, &this->play) != SL_RESULT_SUCCESS){
				this->Destroy();
				throw aumiks::Exc("OpenSLES: Obtaining Play interface failed");
			}

			//get the buffer queue interface
			if((*this->object)->GetInterface(this->object, SL_IID_BUFFERQUEUE, &this->bufferQueue) != SL_RESULT_SUCCESS){
				this->Destroy();
				throw aumiks::Exc("OpenSLES: Obtaining Play interface failed");
			}

			//register callback on the buffer queue
			if((*this->bufferQueue)->RegisterCallback(
					this->bufferQueue,
					&Callback,
					this //context to be passed to the callback
				) != SL_RESULT_SUCCESS)
			{
				this->Destroy();
				throw aumiks::Exc("OpenSLES: Registering callback on the buffer queue failed");
			}
		}
		
		~Player(){
			this->Destroy();
		}
		
		void Destroy(){
			(*this->object)->Destroy(this->object);
		}
		
	private:
		Player(const Player&);
		Player& operator=(const Player&);
	} player;
	
	
	
	/*
	//this callback handler is called every time a buffer finishes playing
	static void Callback(SLAndroidSimpleBufferQueueItf bq, void *context){
		TODO:
		assert(bq == bqPlayerBufferQueue);
		assert(NULL == context);
		// for streaming playback, replace this test by logic to find and fill the next buffer
		if (--nextCount > 0 && NULL != nextBuffer && 0 != nextSize) {
			SLresult result;
			// enqueue another buffer
			result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, nextBuffer, nextSize);
			// the most likely other result is SL_RESULT_BUFFER_INSUFFICIENT,
			// which for this code example would indicate a programming error
			assert(SL_RESULT_SUCCESS == result);
		}
	}
	*/
	
	
	
	//create buffered queue player
	OpenSLESBackend(unsigned bufferSizeFrames, aumiks::E_Format format) :
			outputMix(this->engine),
			player(this->engine, this->outputMix, bufferSizeFrames, format)
	{
		// Set player to playing state
		if((*player.play)->SetPlayState(player.play, SL_PLAYSTATE_PLAYING) != SL_RESULT_SUCCESS){
			throw aumiks::Exc("OpenSLES: Setting player state to PLAYING failed");
		}
	}

public:

	~OpenSLESBackend(){
		// Stop player playing
		SLresult result = (*player.play)->SetPlayState(player.play, SL_PLAYSTATE_STOPPED);
		ASSERT(res == SL_RESULT_SUCCESS);
	}
	
	inline static ting::Ptr<OpenSLESBackend> New(unsigned bufferSizeFrames, aumiks::E_Format format){
		return ting::Ptr<OpenSLESBackend>(
				new OpenSLESBackend(bufferSizeFrames, format)
			);
	}
};

}//~namespace
