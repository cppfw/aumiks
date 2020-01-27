include prorab.mk

this_name := aumiks

this_soname := $(shell cat $(prorab_this_dir)/soname.txt)


this_srcs += $(call prorab-src-dir, .)



this_cxxflags += -Wall # enable all warnings
this_cxxflags += -Wno-comment # no warnings on nested comments
this_cxxflags += -funsigned-char # the 'char' type is unsigned
this_cxxflags += -Wnon-virtual-dtor # warn if base class has non-virtual destructor
this_cxxflags += -fno-operator-names # do not treat 'and', 'bitand','bitor','compl','not','or','xor' as keywords
this_cxxflags += -Werror # treat warnings as errors
this_cxxflags += -Wfatal-errors # stop on first error encountered
this_cxxflags += -fstrict-aliasing # strict aliasing (in order to comply with the c++ standard more strictly)
this_cxxflags += -g # include debugging symbols
this_cxxflags += -std=c++11


ifeq ($(debug),true)
    this_cxxflags += -DDEBUG
    this_objcflags += -DDEBUG
else
    this_cxxflags += -O3
endif


ifeq ($(prorab_os), linux)
    this_cxxflags += -fPIC
    this_ldlibs +=
else ifeq ($(prorab_os), windows)
    this_ldlibs += -lstdc++ -laudout
else ifeq ($(prorab_os), macosx)
    this_cxxflags += -stdlib=libc++ # this is needed to be able to use c++11 std lib
    this_ldlibs += -lc++
endif

this_ldlibs += -lpapki

$(eval $(prorab-build-lib))
