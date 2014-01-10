/* The MIT License:

Copyright (c) 2012-2013 Ivan Gagis

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

#include <ting/Ref.hpp>

#include "SampleBufferFiller.hpp"


namespace aumiks{



/**
 * @brief Base class for effect classes which can be applied to a playing sound.
 * The effects should derive from this class and re-implement the virtual methods
 * which are called to apply the effect to a sound when it is played.
 * The effects can be added to the playing channel.
 */
class Effect : public SampleBufferFiller, public virtual ting::RefCounted{
	friend class aumiks::Channel;
	
	typedef std::list<ting::Ref<aumiks::Effect> > T_EffectsList;
	typedef T_EffectsList::iterator T_EffectsIter;

public:
};


}//~namespace
