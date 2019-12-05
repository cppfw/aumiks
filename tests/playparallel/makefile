include prorab.mk


this_name := tests


this_srcs := main.cpp


this_cxxflags += -Wall
this_cxxflags += -Wno-comment # no warnings on nested comments
this_cxxflags += -Werror
this_cxxflags += -fstrict-aliasing # strict aliasing!!!
this_cxxflags += -g
this_cxxflags += -std=c++11
this_cxxflags += -O3

ifeq ($(debug), true)
    this_cxxflags += -DDEBUG
endif

ifeq ($(os),windows)
    this_ldlibs += $(d)../../src/libaumiks$(soext)
else ifeq ($(os),macosx)
    this_ldlibs += -framework AudioUnit
else ifeq ($(os),linux)
    this_ldlibs += -lpthread -lpulse-simple -lpulse
endif

this_ldlibs += $(abspath $(d)../../src/libaumiks$(soext))

this_ldlibs += -lpogodi -lnitki -laudout -lpapki -lstdc++ -lm

this_no_install := true

$(eval $(prorab-build-app))


define this_rules
test:: $(prorab_this_name)
	@echo running $$^...
	@(cd $(d); LD_LIBRARY_PATH=../../src $$^)
endef
$(eval $(this_rules))


# add dependency on libaumiks
ifeq ($(os),windows)
    $(d)libaumiks$(soext): $(abspath $(d)../../src/libaumiks$(soext))
	@cp $< $@

    $(prorab_this_name): $(d)libaumiks$(soext)

    define this_rules
        clean::
		@rm -f $(d)libaumiks$(soext)
    endef
    $(eval $(this_rules))
else
    $(prorab_this_name): $(abspath $(d)../../src/libaumiks$(soext))
endif


$(eval $(call prorab-try-include,$(d)../../src/makefile))
