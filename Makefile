CC = gcc
CFLAGS = -g -std=c11 -Wall -O2 -D_HAVE_SELECT -D_XOPEN_SOURCE=700
RM = rm -f
LD = gcc
MKDIR = mkdir

all: obj ws2300

clean:
	$(RM) *.o ws2300

ws2300: obj/main.o obj/serial.o obj/ws2300.o obj/decoder.o
	$(LD) -o $@ $+ -lm

obj:
	$(MKDIR) $@

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -o $@ -c $<
