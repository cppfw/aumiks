/* The MIT License:

Copyright (c) 2009-2012 Ivan Gagis

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

// Home page: http://aumiks.googlecode.com

/**
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once

#include <ting/Array.hpp>

#include <SLES/OpenSLES.h>

#if M_OS_NAME == M_OS_NAME_ANDROID
#	include <SLES/OpenSLES_Android.h>
#endif

#include "../Player.hpp"


namespace{

class OpenSLESBackend : public audout::Player{
	
	struct Engine{
		SLObjectItf object; //object
		SLEngineItf engine; //Engine interface
		
		Engine(){
			//create engine object
			if(slCreateEngine(&this->object, 0, NULL, 0, NULL, NULL) != SL_RESULT_SUCCESS){
				throw ting::Exc("OpenSLES: Creating engine object failed");
			}
			
			//realize the engine
			if((*this->object)->Realize(this->object, SL_BOOLEAN_FALSE) != SL_RESULT_SUCCESS){
				this->Destroy();
				throw ting::Exc("OpenSLES: Realizing engine object failed");
			}
			
			//get the engine interface, which is needed in order to create other objects
			if((*this->object)->GetInterface(this->object, SL_IID_ENGINE, &this->engine) != SL_RESULT_SUCCESS){
				this->Destroy();
				throw ting::Exc("OpenSLES: Obtaining Engine interface failed");
			}
		}
		
		~Engine()throw(){
			this->Destroy();
		}
		
		void Destroy()throw(){
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
				throw ting::Exc("OpenSLES: Creating output mix object failed");
			}
			
			// realize the output mix
			if((*this->object)->Realize(this->object, SL_BOOLEAN_FALSE) != SL_RESULT_SUCCESS){
				this->Destroy();
				throw ting::Exc("OpenSLES: Realizing output mix object failed");
			}
		}
		
		~OutputMix()throw(){
			this->Destroy();
		}
		
		void Destroy()throw(){
			(*this->object)->Destroy(this->object);
		}
		
	private:
		OutputMix(const OutputMix&);
		OutputMix& operator=(const OutputMix&);
	} outputMix;
	
	
	
	struct Player{
		OpenSLESBackend& backend;
		
		SLObjectItf object;
		SLPlayItf play;
#if M_OS_NAME == M_OS_NAME_ANDROID
		SLAndroidSimpleBufferQueueItf
#else
		SLBufferQueueItf
#endif
				bufferQueue;
		
		ting::StaticBuffer<ting::Array<ting::u8>, 2> bufs;
	
	
		//this callback handler is called every time a buffer finishes playing
		static void Callback(
#if M_OS_NAME == M_OS_NAME_ANDROID
				SLAndroidSimpleBufferQueueItf queue,
				void *context
#else
				SLBufferQueueItf queue,
				SLuint32 eventFlags,
				const void *buffer,
				SLuint32 bufferSize,
				SLuint32 dataUsed,
				void *context
#endif
			)
		{
//			TRACE(<< "OpenSLESBackend::Player::Callback(): invoked" << std::endl)
			
			ASSERT(context)
			Player* player = static_cast<Player*>(context);
			
#if M_OS_NAME == M_OS_NAME_ANDROID
#else
			ASSERT(buffer == player->bufs[0].Begin())
			ASSERT(bufferSize == player->bufs[0].Size())
			ASSERT(dataUsed <= bufferSize)
#endif
			
			ASSERT(player->bufs.Size() == 2)
			std::swap(player->bufs[0], player->bufs[1]); //swap buffers, the 0th one is the buffer which is currently playing
			
#if M_OS_NAME == M_OS_NAME_ANDROID
			SLresult res = (*queue)->Enqueue(queue, player->bufs[0].Begin(), player->bufs[0].Size());
#else
			SLresult res = (*queue)->Enqueue(queue, player->bufs[0].Begin(), player->bufs[0].Size(), SL_BOOLEAN_FALSE);
#endif
			ASSERT(res == SL_RESULT_SUCCESS)
			
			//fill the second buffer to be enqueued next time the callback is called
			player->backend.FillPlayBuf(player->bufs[1]);
		}
		
		
		
		Player(OpenSLESBackend& backend, Engine& engine, OutputMix& outputMix, unsigned bufferSizeFrames, aumiks::E_Format format) :
				backend(backend)
		{
			//Allocate play buffers of required size
			{
				size_t bufSize = bufferSizeFrames * aumiks::BytesPerFrame(format);
				for(ting::Array<ting::u8>* i = this->bufs.Begin(); i != this->bufs.End(); ++i){
					i->Init(bufSize);
				}
				ASSERT(this->bufs.Size() == 2)
				//Initialize the first buffer with 0's, since playing will start from the first buffer
				memset(this->bufs[0].Begin(), 0, this->bufs[0].Size());
			}
			
			//========================
			// configure audio source
#if M_OS_NAME == M_OS_NAME_ANDROID
			SLDataLocator_AndroidSimpleBufferQueue bufferQueueStruct = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2}; //2 buffers in queue
#else
			SLDataLocator_BufferQueue bufferQueueStruct = {SL_DATALOCATOR_BUFFERQUEUE, 2}; //2 buffers in queue
#endif
			
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
			
			SLDataSource audioSourceStruct = {&bufferQueueStruct, &audioFormat};

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
				throw ting::Exc("OpenSLES: Creating player object failed");
			}
			
			//realize the player
			if((*this->object)->Realize(this->object, SL_BOOLEAN_FALSE) != SL_RESULT_SUCCESS){
				this->Destroy();
				throw ting::Exc("OpenSLES: Realizing player object failed");
			}

			//get the play interface
			if((*this->object)->GetInterface(this->object, SL_IID_PLAY, &this->play) != SL_RESULT_SUCCESS){
				this->Destroy();
				throw ting::Exc("OpenSLES: Obtaining Play interface failed");
			}

			//get the buffer queue interface
			if((*this->object)->GetInterface(this->object, SL_IID_BUFFERQUEUE, &this->bufferQueue) != SL_RESULT_SUCCESS){
				this->Destroy();
				throw ting::Exc("OpenSLES: Obtaining Play interface failed");
			}

			//register callback on the buffer queue
			if((*this->bufferQueue)->RegisterCallback(
					this->bufferQueue,
					&Callback,
					this //context to be passed to the callback
				) != SL_RESULT_SUCCESS)
			{
				this->Destroy();
				throw ting::Exc("OpenSLES: Registering callback on the buffer queue failed");
			}
		}
		
		~Player()throw(){
			this->Destroy();
		}
		
		void Destroy()throw(){
			(*this->object)->Destroy(this->object);
		}
		
	private:
		Player(const Player&);
		Player& operator=(const Player&);
	} player;
	
	

	//override
	void SetPaused(bool pause){
		if(pause){
			(*player.play)->SetPlayState(player.play, SL_PLAYSTATE_STOPPED);
		}else{
			(*player.play)->SetPlayState(player.play, SL_PLAYSTATE_PLAYING);
		}
	}
	
public:
	//create buffered queue player
	OpenSLESBackend(unsigned bufferSizeFrames, aumiks::E_Format format) :
			outputMix(this->engine),
			player(*this, this->engine, this->outputMix, bufferSizeFrames, format)
	{
//		TRACE(<< "OpenSLESBackend::OpenSLESBackend(): Starting player" << std::endl)
		// Set player to playing state
		if((*player.play)->SetPlayState(player.play, SL_PLAYSTATE_PLAYING) != SL_RESULT_SUCCESS){
			throw ting::Exc("OpenSLES: Setting player state to PLAYING failed");
		}
		
		//Enqueue the first buffer for playing, otherwise it will not start playing
#if M_OS_NAME == M_OS_NAME_ANDROID
		SLresult res = (*this->player.bufferQueue)->Enqueue(this->player.bufferQueue, this->player.bufs[0].Begin(), this->player.bufs[0].Size());
#else
		SLresult res = (*this->player.bufferQueue)->Enqueue(this->player.bufferQueue, this->player.bufs[0].Begin(), this->player.bufs[0].Size(), SL_BOOLEAN_FALSE);
#endif
		if(res != SL_RESULT_SUCCESS){
			throw ting::Exc("OpenSLES: unable to enqueue");
		}
	}

	
	
	~OpenSLESBackend()throw(){
		// Stop player playing
		SLresult res = (*player.play)->SetPlayState(player.play, SL_PLAYSTATE_STOPPED);
		ASSERT(res == SL_RESULT_SUCCESS);
		
		//TODO: make sure somehow that the callback will not be called anymore
	}
};

}//~namespace
