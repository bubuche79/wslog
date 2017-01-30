bin_PROGRAMS = \
	check_aggregate \
	check_nybble \
	check_util

check_aggregate_SOURCES = check_aggregate.c
check_aggregate_CPPFLAGS = -I../src/common -I../src/lib
check_aggregate_LDADD = -L../src/lib/libws -lws -lcheck

check_nybble_SOURCES = check_nybble.c
check_nybble_CPPFLAGS = -I../src/lib
check_nybble_LDADD = -L../src/lib/libws -lws -lcheck

check_util_SOURCES = check_util.c
check_util_CPPFLAGS = -I../src/common -I../src/lib
check_util_LDADD = -L../src/lib/libws -lws -lcheck

all: all_checks

all_checks:
	./check_aggregate
	./check_nybble
	./check_util
