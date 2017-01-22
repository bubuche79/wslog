lib_LTLIBRARIES = libws.la

libws_la_SOURCES = \
	bitfield.c \
	conf.c \
	err.c \
	nybble.c \
	util.c \
	serial.c \
	bitfield.h \
	conf.h \
	err.h \
	log.h \
	nybble.h \
	util.h \
	serial.h

if HAVE_WS23XX
libws_la_SOURCES += \
	ws23xx/archive.c \
	ws23xx/decoder.c \
	ws23xx/ws23xx.c \
	ws23xx/archive.h \
	ws23xx/decoder.h \
	ws23xx/ws23xx.h
endif

libws_la_CFLAGS = \
	-std=c11 -pedantic -D_XOPEN_SOURCE=700 -Wall -Werror -I../../common -I..

libws_la_LIBADD = \
	-lm
