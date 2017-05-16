#pragma once


#include "Source.hpp"



namespace aumiks{



/**
 * @brief Base class for sounds.
 * A sound object is an object which holds all the initial data required to play a particular sound.
 * Sound object is normally used to create instances of a source to play that sound.
 */
class Sound : virtual public utki::Shared{
protected:
	Sound(){}
public:
	
	virtual ~Sound()noexcept{}

	//TODO: doxygen
	virtual std::shared_ptr<aumiks::Source> createSource(std::uint32_t frequency = 0)const = 0;
};

}
