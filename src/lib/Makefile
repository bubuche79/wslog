include		../Makefile.master

#
# List of all SUBDIRS (libraries) to build
#
SUBDIRS =	libws

#
# Targets and sub-directories to enter and build
#
TARGETS =	all clean clobber install

.PHONY: $(TARGETS) $(SUBDIRS)

$(TARGETS): $(SUBDIRS)

$(SUBDIRS):
	@cd $@; $(MAKE) $(MAKECMDGOALS)
