CC := gcc

CFLAGS := -std=c99 \
	-Wall -Wextra -Wno-missing-field-initializers \
	-O2 \
	-DHAVE_SELECT -D_XOPEN_SOURCE=700 \
	-DPROGNAME=\"$(PKG_NAME)\" -DVERSION=\"$(PKG_VERSION)\" \
	$(CFLAGS)

LDFLAGS := -L../src/lib/libws

all: check_nybble

check: check_nybble
	LD_LIBRARY_PATH=../src/lib/libws ./check_nybble

check_nybble: check_nybble.o
	$(CC) $(LDFLAGS) -o $@ $+ -lws -lcheck

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -I../src/lib -c $<

clean:
	$(RM) -r *.o
