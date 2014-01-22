/* The MIT License:

Copyright (c) 2014 Ivan Gagis

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

#include <ting/types.hpp>
#include <ting/Buffer.hpp>
#include <ting/Ptr.hpp>

#include "AudioFormat.hpp"
#include "PlayerListener.hpp"

namespace audout{

//TODO: doxygen
class Player {
	PlayerListener* listener;
	
private:
	Player(const Player&);
	Player& operator=(const Player&);
	
protected:
	Player(PlayerListener* listener);
	
	inline const PlayerListener* Listener()throw(){
		return this->listener;
	}
public:
	virtual ~Player()throw();
	
	virtual void Start() = 0;
	
	virtual void SetPaused(bool pause) = 0;
	
	static ting::Ptr<Player> CreatePlayer(AudioFormat outputFormat, ting::u32 bufSizeInFrames, PlayerListener* listener);
};

}//~namespace
