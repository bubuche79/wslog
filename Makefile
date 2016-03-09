PKG_NAME := ws23xx
PKG_VERSION := 0.4

CC := gcc

CFLAGS := -std=c99 \
	-Wall -Wextra -Wno-missing-field-initializers \
	-O2 \
	-DHAVE_SELECT -D_XOPEN_SOURCE=700 \
	-DPROGNAME=\"$(PKG_NAME)\" -DVERSION=\"$(PKG_VERSION)\" \
	$(CFLAGS)

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

all: .deps ws2300

clean:
	$(RM) -r .deps/ src/*.o ws2300

ws2300: $(OBJS)
	$(CC) -o $@ $+ -lm -lcurl

.deps:
	$(MKDIR) $@

src/%.o: src/%.c
	$(CC) $(CFLAGS) -MT $@ -MD -MP -MF .deps/$(notdir $@).Tpo -c -o $@ -Isrc $<
	$(MV) .deps/$(notdir $@).Tpo .deps/$(notdir $@).Po

install:
	install -d $(DESTDIR)/usr/bin
	install ws2300 $(DESTDIR)/usr/bin

dist:
	$(GIT) archive --prefix=$(PKG_NAME)-$(PKG_VERSION)/ -o $(PKG_NAME)-$(PKG_VERSION).tar.gz HEAD
