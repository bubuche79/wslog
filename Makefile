PKG_NAME := ws23xx
PKG_VERSION := 0.4

CC := gcc

CFLAGS := \
	-pedantic \
	-std=c11 \
	-Wall -Wextra -Wno-missing-field-initializers \
	-Wstrict-prototypes -Wmissing-prototypes -Wshadow \
	-O2 \
	-DHAVE_SELECT -D_XOPEN_SOURCE=700 \
	-DPROGNAME=\"$(PKG_NAME)\" -DVERSION=\"$(PKG_VERSION)\" \
	$(CFLAGS)

LDFLAGS := \
	$(LDFLAGS)

OBJS := \
	src/core/bitfield.o \
	src/core/nybble.o \
	src/decoder.o \
	src/history.o \
	src/main.o \
	src/serial.o \
	src/util.o \
	src/ws2300.o \
	src/wunder.o

GIT := git
TAR := tar

RM := rm -f
MKDIR := mkdir
MV := mv
TOUCH := touch
TAR := tar
INSTALL := install

SUBDIRS = \
	src/core \
	src

SUBDIRS_DEPS = \
	$(addsuffix /.deps, $(SUBDIRS))

all: $(SUBDIRS_DEPS) ws2300

clean:
	$(RM) -r $(SUBDIRS_DEPS) src/*.o ws2300

ws2300: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $+ -lcurl -lm

$(SUBDIRS_DEPS):
	$(MKDIR) $@

%.o: %.c
	$(CC) $(CFLAGS) -MT $@ -MD -MF $(dir $@).deps/$(notdir $@).Tpo -c -o $@ -Isrc $<
	$(MV) $(dir $@).deps/$(notdir $@).Tpo $(dir $@).deps/$(notdir $@).Po

install:
	$(INSTALL) -d $(DESTDIR)/usr/bin
	$(INSTALL) ws2300 $(DESTDIR)/usr/bin

check:
	$(MAKE) -C tests check

dist:
	$(GIT) archive --prefix=$(PKG_NAME)-$(PKG_VERSION)/ -o $(PKG_NAME)-$(PKG_VERSION).tar.gz HEAD

# Dependencies
include $(wildcard $(addsuffix /*.Po, $(SUBDIRS_DEPS)))
