check_PROGRAMS = \
	check_build

check_build_SOURCES = \
	check.c \
	check_aggregate.c \
	check_crc_ccitt.c \
	check_nybble.c \
	check_util.c \
	check_vantage.c

check_build_CPPFLAGS = -I../src/common -I../src

check_build_LDADD = -L../src/libws -lws -lcheck

check:
	./check_build
