PKG_NAME := ws23xx
PKG_VERSION := 0.5

#
# List of all SUBDIRS to build
#
SUBDIRS =	src

#
# Targets and sub-directories to enter and build
#
TARGETS =	all clean clobber install

.PHONY: $(TARGETS) $(SUBDIRS)

$(TARGETS): $(SUBDIRS)

$(SUBDIRS):
	@cd $@; $(MAKE) $(MAKECMDGOALS)

dist:
	git archive --prefix=$(PKG_NAME)-$(PKG_VERSION)/ -o $(PKG_NAME)-$(PKG_VERSION).tar.gz HEAD

#
# Explicit dependencies for parallel builds
#
test:		src
