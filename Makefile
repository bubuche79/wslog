CC = gcc
CFLAGS = -g -std=c11 -Wall -O2 -DDEBUG=1 -D_XOPEN_SOURCE=700
RM = rm -f
LD = gcc

all: ws2300

clean:
	$(RM) *.o ws2300

ws2300: main.o serial.o ws2300.o
	$(LD) -o $@ main.o serial.o ws2300.o

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<
