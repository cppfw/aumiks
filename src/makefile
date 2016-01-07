include prorab.mk

this_name := audout

this_soname := $(shell cat $(prorab_this_dir)/soname.txt)


#this_srcs += aumiks/Channel.cpp
#this_srcs += aumiks/Effect.cpp
#this_srcs += aumiks/Lib.cpp
#this_srcs += aumiks/MixChannel.cpp
#this_srcs += aumiks/util.cpp
this_srcs += aumiks/WavSound.cpp



this_cflags += -Wall #enable all warnings
this_cflags += -Wno-comment #no warnings on nested comments
this_cflags += -funsigned-char #the 'char' type is unsigned
this_cflags += -Wnon-virtual-dtor #warn if base class has non-virtual destructor
this_cflags += -fno-operator-names #do not treat 'and', 'bitand','bitor','compl','not','or','xor' as keywords
this_cflags += -Werror #treat warnings as errors
this_cflags += -Wfatal-errors # stop on first error encountered
this_cflags += -fstrict-aliasing #strict aliasing (in order to comply with the c++ standard more strictly)
this_cflags += -g #include debugging symbols
                #-P -E #-Wreorder
                #-O3 -funroll-loops -fomit-frame-pointer
this_cflags += -std=c++11


ifeq ($(debug),true)
    this_cflags += -DDEBUG
#    this_cflags += -DM_MORDA_RENDER_WIDGET_BORDERS
    this_objcflags += -DDEBUG
endif


ifeq ($(prorab_os), linux)
    this_cflags += -fPIC
    this_ldlibs += -lpthread
else ifeq ($(prorab_os), windows)
    this_cflags += -Wno-format #due to bugs in some g++ versions it warns about format when not needed, so disable format warnings
    this_cflags += -I/usr/include -I/usr/local/include/
    this_ldflags += -L/usr/local/lib -L/usr/lib

    #NOTE: order matters in MinGW, libs referenced by other libs should go after those libs which reference them.
    this_ldlibs += #-lfreetype -lstob -ljpeg -lpng -lz -lopengl32 -lglew32 -lgdi32

    #WORKAROUND for MinGW bug:
    #this_cflags += -D__STDC_FORMAT_MACROS
else ifeq ($(prorab_os), macosx)
    this_cflags += -stdlib=libc++ #this is needed to be able to use c++11 std lib
    this_ldlibs += -lc++
    #this_ldlibs += -framework Cocoa -framework OpenGL -lpng -lfreetype -ljpeg -lstob -lGLEW
    
    #this_mm_obj := $(prorab_this_dir)$(prorab_obj_dir)morda/glue/macosx/main.o

    #define this_rules
    #    $(this_mm_obj): $(prorab_this_dir)morda/glue/macosx/main.mm
#		@echo Compiling $$<...
#		$(prorab_echo)mkdir -p $$(dir $$@)
#		$(prorab_echo)$(CC) -ObjC++ -std=c++11 -c -o "$$@" $(this_objcflags) $$<
#    endef
#    $(eval $(this_rules))
endif

this_ldlibs += 

$(eval $(prorab-build-lib))
