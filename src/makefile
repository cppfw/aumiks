include prorab.mk

$(eval $(call prorab-config, ../config))

this_name := aumiks

this_soname := $(shell cat $(d)soname.txt)

this_srcs += $(call prorab-src-dir, .)

ifeq ($(os), linux)
else ifeq ($(os), windows)
    this_ldlibs += -laudout
else ifeq ($(os), macosx)
endif

this_ldlibs += -lpapki

$(eval $(prorab-build-lib))
