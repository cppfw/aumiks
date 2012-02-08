subdirs :=
subdirs += src
subdirs += tests
subdirs += docs



include ./targets.mk

$(possible_targets_list):
	@$(MAKE) $(subdirs) target=$@

.PHONY: $(subdirs)
$(subdirs):
	@$(MAKE) -C $@ $(target)


