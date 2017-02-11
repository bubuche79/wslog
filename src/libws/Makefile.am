include ../include.mk

lib_LTLIBRARIES = libws.la

libws_la_SOURCES = \
	aggregate.c \
	bitfield.c \
	conf.c \
	err.c \
	nybble.c \
	util.c \
	serial.c \
	aggregate.h \
	bitfield.h \
	conf.h \
	err.h \
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

libws_la_CPPFLAGS = \
	$(inc_CPPFLAGS) -I../common -I..

libws_la_CFLAGS = \
	$(inc_CFLAGS)

libws_la_LIBADD = \
	-lm
