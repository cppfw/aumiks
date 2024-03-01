include prorab.mk
include prorab-license.mk

$(eval $(call prorab-config, ../config))

this_name := aumiks

this_soname := $(shell cat $(d)soname.txt)

this_srcs += $(call prorab-src-dir, .)

this_ldlibs += -laudout
this_ldlibs += -lpapki

$(eval $(prorab-build-lib))

this_src_dir := $(this_name)
this_license_file := ../LICENSE
$(eval $(prorab-license))
