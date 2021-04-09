include prorab.mk

$(eval $(call prorab-config, ../../config))

this_name := tests

this_srcs := $(call prorab-src-dir, .)

this_libaumiks := $(d)../../src/out/$(c)/libaumiks$(dot_so)

ifeq ($(os),macosx)
    this_ldlibs += -framework AudioUnit
else ifeq ($(os),linux)
    this_ldlibs += -lpthread -lpulse-simple -lpulse
endif

this_ldlibs += $(abspath $(this_libaumiks))

this_ldlibs += -laudout -lpapki -lutki

this_no_install := true

$(eval $(prorab-build-app))

define this_rules
run:: $(prorab_this_name)
$(.RECIPEPREFIX)@echo running $$^...
$(.RECIPEPREFIX)$(a)(cd $(d); LD_LIBRARY_PATH=../../src/out/$(c) $$^)
endef
$(eval $(this_rules))

# add dependency on libaumiks
ifeq ($(os),windows)
    $(d)out/$(c)/libaumiks$(dot_so): $(abspath $(this_libaumiks))
	@cp $< $@

    $(prorab_this_name): $(d)out/$(c)/libaumiks$(dot_so)

    define this_rules
        clean::
$(.RECIPEPREFIX)$(a)rm -f $(d)out/$(c)/libaumiks$(dot_so)
    endef
    $(eval $(this_rules))
else
    $(prorab_this_name): $(abspath $(this_libaumiks))
endif


$(eval $(call prorab-try-include,$(d)../../src/makefile))
