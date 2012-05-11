subdirs :=
#TODO:
#subdirs += playwav
#subdirs += playparallel
#subdirs += effects
subdirs += sinesynth



include ../targets.mk

$(possible_targets_list):
	@$(MAKE) $(subdirs) target=$@

.PHONY: $(subdirs)
$(subdirs):
	@$(MAKE) -C $@ $(target)


