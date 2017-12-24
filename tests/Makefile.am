bin_PROGRAMS = \
	check

check_SOURCES = \
	check.c \
	check_aggregate.c \
	check_crc_ccitt.c \
	check_nybble.c \
	check_util.c

check_CPPFLAGS = -I../src/common -I../src

check_LDADD = -L../src/libws -lws -lcheck

all: tests

tests:
	./check
