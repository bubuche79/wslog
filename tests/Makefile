CC := gcc

CFLAGS := -std=c99 \
	-Wall -Wextra -Wno-missing-field-initializers \
	-O2 \
	-DHAVE_SELECT -D_XOPEN_SOURCE=700 \
	-DPROGNAME=\"$(PKG_NAME)\" -DVERSION=\"$(PKG_VERSION)\" \
	$(CFLAGS)

test: check_nybble

check_nybble: check_nybble.o ../src/core/nybble.o
	$(CC) -o $@ $+ -lcheck

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -I../src -c $<

clean:
	$(RM) -r *.o
