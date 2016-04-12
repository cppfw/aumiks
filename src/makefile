include prorab.mk

this_name := aumiks

this_soname := $(shell cat $(prorab_this_dir)/soname.txt)


this_srcs += aumiks/WavSound.cpp
this_srcs += aumiks/Input.cpp
this_srcs += aumiks/Reframer.cpp
this_srcs += aumiks/Mixer.cpp



this_cxxflags += -Wall #enable all warnings
this_cxxflags += -Wno-comment #no warnings on nested comments
this_cxxflags += -funsigned-char #the 'char' type is unsigned
this_cxxflags += -Wnon-virtual-dtor #warn if base class has non-virtual destructor
this_cxxflags += -fno-operator-names #do not treat 'and', 'bitand','bitor','compl','not','or','xor' as keywords
this_cxxflags += -Werror #treat warnings as errors
this_cxxflags += -Wfatal-errors # stop on first error encountered
this_cxxflags += -fstrict-aliasing #strict aliasing (in order to comply with the c++ standard more strictly)
this_cxxflags += -g #include debugging symbols
this_cxxflags += -O3
this_cxxflags += -std=c++11


ifeq ($(debug),true)
    this_cxxflags += -DDEBUG
    this_objcflags += -DDEBUG
endif


ifeq ($(prorab_os), linux)
    this_cxxflags += -fPIC
    this_ldlibs += -lpthread
else ifeq ($(prorab_os), windows)
    this_cxxflags += -Wno-format #due to bugs in some g++ versions it warns about format when not needed, so disable format warnings
    this_cxxflags += -I/usr/include -I/usr/local/include/
    this_ldflags += -L/usr/local/lib -L/usr/lib

    #NOTE: order matters in MinGW, libs referenced by other libs should go after those libs which reference them.
    this_ldlibs += #-lfreetype -lstob -ljpeg -lpng -lz -lopengl32 -lglew32 -lgdi32

    #WORKAROUND for MinGW bug:
    #this_cxxflags += -D__STDC_FORMAT_MACROS
else ifeq ($(prorab_os), macosx)
    this_cxxflags += -stdlib=libc++ #this is needed to be able to use c++11 std lib
    this_ldlibs += -lc++
    #this_ldlibs += -framework Cocoa -framework OpenGL -lpng -lfreetype -ljpeg -lstob -lGLEW
endif

this_ldlibs += -lpapki

$(eval $(prorab-build-lib))
