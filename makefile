include prorab.mk

$(eval $(prorab-build-subdirs))

this_soname_dependency := $(prorab_this_dir)src/soname.txt
this_soname := $(shell cat $(this_soname_dependency))

$(eval $(prorab-build-deb))
