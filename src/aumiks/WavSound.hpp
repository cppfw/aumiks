/**
 * @author Ivan Gagis <igagis@gmail.com>
 */


#pragma once


#include <papki/File.hpp>

#include "Sound.hpp"



namespace aumiks{



class WavSound : public aumiks::Sound{
	
	std::uint8_t chans;
	std::uint32_t freq;
	
protected:
	WavSound(std::uint8_t chans, std::int32_t freq) :
			chans(chans),
			freq(freq)
	{}

public:
	inline std::uint8_t NumChannels()const throw(){
		return this->chans;
	}
	
	inline std::uint32_t SamplingRate()const throw(){
		return this->freq;
	}

	class Source : public aumiks::Source{
		Source(const Source&);
		Source& operator=(const Source&);
	protected:
		Source(aumiks::Output& output) :
				aumiks::Source(output)
		{}
	public:
		
		//TODO:
	};
	
	virtual std::shared_ptr<Source> CreateWavSource()const = 0;
	
	std::shared_ptr<aumiks::Source> CreateSource()const override{
		return this->CreateWavSource();
	}
	
	//TODO:
//	inline Ref<WavSound::Channel> Play(unsigned numLoops = 1)const{
//		Ref<WavSound::Channel> ret = this->CreateWavChannel();
//		ret->Play(numLoops);
//		return ret;
//	}

	static std::shared_ptr<WavSound> Load(const std::string& fileName);
	static std::shared_ptr<WavSound> Load(papki::File& fi);
};



}
