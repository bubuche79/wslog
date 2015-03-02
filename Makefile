PKG_NAME := ws23xx
PKG_VERSION := 0.1

CC := gcc

CFLAGS := -std=c99 \
	-Wall -Wextra -Wno-missing-field-initializers \
	-O2 \
	-DHAVE_SELECT -D_XOPEN_SOURCE=700 \
	-DPROGNAME=\"$(PKG_NAME)\" -DVERSION=\"$(PKG_VERSION)\" \
	$(CFLAGS)

TAR := tar

RM := rm -f
MKDIR := mkdir
MV := mv
TOUCH := touch
TAR := tar

all: .deps ws2300

clean:
	$(RM) -r .deps/ src/*.o ws2300

ws2300: src/main.o src/history.o src/serial.o src/ws2300.o src/decoder.o src/util.o
	$(CC) -o $@ $+ -lm

.deps:
	$(MKDIR) $@

src/%.o: src/%.c
	$(CC) $(CFLAGS) -MT $@ -MD -MP -MF .deps/$(notdir $@).Tpo -c -o $@ $<
	$(MV) .deps/$(notdir $@).Tpo .deps/$(notdir $@).Po

install:
	install -d $(DESTDIR)/usr/bin
	install ws2300 $(DESTDIR)/usr/bin

dist:
	$(TAR) czf ws23xx-0.1.tar.gz --transform 's,^,ws23xx-0.1/,' -- src/*.{c,h} Makefile
