#include <iostream>
#include <functional>

#include <aumiks/speakers.hpp>

int main(int argc, const char** argv){
	std::function<void()> f = [](){
		aumiks::speakers sink(audout::rate::hz_22050);
		sink.start();
	};

	if(f){
    	std::cout << "Hello aumiks!" << std::endl;
	}

    return 0;
}
