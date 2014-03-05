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

#include "Source.hpp"


// Home page: http://aumiks.googlecode.com

/**
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once

#include "Source.hpp"

#include <list>

namespace aumiks{

//TODO: doxygen
template <class T_Sample, ting::u8 num_channels> class Mixer : public Source<T_Sample, num_channels>{
	Mixer(const Mixer&);
	Mixer& operator=(const Mixer&);
	
	typedef std::list<ting::Ref<Source<T_Sample, num_channels> > > T_List;
	
	T_List sources;
	
	bool isPersistent;
	
	Mixer(bool isPersistent) :
			isPersistent(isPersistent)
	{}
public:
	
	//override
	bool FillSampleBuffer(const ting::Buffer<T_Sample>& buf)throw(){
		//TODO:
	}
	
	void AddSource(const ting::Ref<Source<T_Sample, num_channels> >& src){
		//TODO:
	}
	
	static ting::Ref<Mixer> New(bool isPersistent = false){
		return ting::Ref<Mixer>(new Mixer(isPersistent));
	}
};

}
