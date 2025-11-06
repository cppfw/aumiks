include prorab.mk
include prorab-test.mk
include prorab-clang-format.mk

$(eval $(call prorab-config, ../../config))

this_name := tests

this_srcs := $(call prorab-src-dir, .)

ifeq ($(os),macosx)
    this_ldlibs += -framework AudioUnit
else ifeq ($(os),linux)
    this_ldlibs += -l pthread
    this_ldlibs += -l pulse-simple
    this_ldlibs += -l pulse
endif

this_ldlibs += -l audout$(this_dbg)
this_ldlibs += -l fsif$(this_dbg)
this_ldlibs += -l utki$(this_dbg)

this_ldlibs += -l m

this_ldlibs += ../../src/out/$(c)/libaumiks$(this_dbg)$(dot_so)

this_no_install := true

$(eval $(prorab-build-app))

this_test_cmd := $(prorab_this_name)
this_test_deps := $(prorab_this_name)
this_test_ld_path := ../../src/out/$(c)
this_run_name := $(notdir $(abspath $(d)))
$(eval $(prorab-run))

this_src_dir := .
$(eval $(prorab-clang-format))

$(eval $(call prorab-include, ../../src/makefile))
