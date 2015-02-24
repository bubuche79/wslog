CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -Wno-missing-field-initializers -O2 -DHAVE_SELECT -D_XOPEN_SOURCE=700 $(CFLAGS)

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
