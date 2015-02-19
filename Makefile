CC = gcc
CFLAGS = -g -std=c11 -Wall -O2 -D_HAVE_SELECT -D_XOPEN_SOURCE=700
RM = rm -f
LD = gcc

all: ws2300

clean:
	$(RM) *.o ws2300

ws2300: main.o serial.o ws2300.o convert.o
	$(LD) -o $@ $+ -lm

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<
