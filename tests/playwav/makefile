include prorab.mk
include prorab-test.mk

$(eval $(call prorab-config, ../../config))

this_name := tests

this_srcs := $(call prorab-src-dir, .)

this_libaumiks := ../../src/out/$(c)/libaumiks$(dot_so)

ifeq ($(os),macosx)
    this_ldlibs += -framework AudioUnit
else ifeq ($(os),linux)
    this_ldlibs += -lpthread -lpulse-simple -lpulse
endif

this_ldlibs += $(this_libaumiks)

this_ldlibs += -laudout -lpapki -lutki

this_no_install := true

$(eval $(prorab-build-app))

$(eval $(call prorab-depend, $(prorab_this_name), $(this_libaumiks)))

this_run_name := $(notdir $(abspath $(d)))
this_test_cmd := $(prorab_this_name)
this_test_deps := $(prorab_this_name)
this_test_ld_path := $(dir $(this_libaumiks))
$(eval $(prorab-run))

$(eval $(call prorab-include, ../../src/makefile))
