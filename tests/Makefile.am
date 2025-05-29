TESTS = check_build

check_PROGRAMS = \
	check_build

check_build_SOURCES = \
	check.c \
	check_aggregate.c \
	check_cli.c \
	check_crc_ccitt.c \
	check_nybble.c \
	check_util.c \
	suites.h

if USE_VANTAGE
check_build_SOURCES += \
	check_vantage.c
endif

check_build_CPPFLAGS = \
	-I../src \
	@CHECK_CFLAGS@

check_build_LDADD = \
	-L../src/libws -lws \
	@CHECK_LIBS@
