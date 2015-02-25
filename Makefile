CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -Wno-missing-field-initializers -O2 -DHAVE_SELECT -D_XOPEN_SOURCE=700 $(CFLAGS)
TAR := tar

RM = rm -f
MKDIR = mkdir

all: obj ws2300

clean:
	$(RM) -r obj ws2300

ws2300: obj/main.o obj/history.o obj/serial.o obj/ws2300.o obj/decoder.o obj/util.o
	$(CC) -o $@ $+ -lm

obj:
	$(MKDIR) $@

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -o $@ -c $<

install:
	install -d $(DESTDIR)/usr/bin
	install ws2300 $(DESTDIR)/usr/bin

package:
	$(TAR) czf ws23xx-0.1.tar.gz --transform 's,^,ws23xx-0.1/,' -- src packages Makefile
