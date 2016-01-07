/**
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once

#include <utki/Shared.hpp>

#include "SampleBufferFiller.hpp"


namespace aumiks{



/**
 * @brief Base class for effect classes which can be applied to a playing sound.
 * The effects should derive from this class and re-implement the virtual methods
 * which are called to apply the effect to a sound when it is played.
 * The effects can be added to the playing channel.
 */
class Effect : public SampleBufferFiller, public virtual utki::Shared{
	friend class aumiks::Channel;
	
	typedef std::list<std::shared_ptr<aumiks::Effect> > T_EffectsList;
	typedef T_EffectsList::iterator T_EffectsIter;

public:
};


}//~namespace
