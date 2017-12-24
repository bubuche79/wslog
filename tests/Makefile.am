bin_PROGRAMS = \
	check_aggregate \
	check_crc_ccitt \
	check_nybble \
	check_util

check_aggregate_SOURCES = check_aggregate.c
check_aggregate_CPPFLAGS = -I../src/common -I../src
check_aggregate_LDADD = -L../src/libws -lws -lcheck

check_crc_ccitt_SOURCES = check_crc_ccitt.c
check_crc_ccitt_CPPFLAGS = -I../src
check_crc_ccitt_LDADD = -L../src/libws -lws -lcheck

check_nybble_SOURCES = check_nybble.c
check_nybble_CPPFLAGS = -I../src
check_nybble_LDADD = -L../src/libws -lws -lcheck

check_util_SOURCES = check_util.c
check_util_CPPFLAGS = -I../src
check_util_LDADD = -L../src/libws -lws -lcheck

all: all_checks

all_checks:
	./check_aggregate
	./check_crc_ccitt
	./check_nybble
	./check_util
