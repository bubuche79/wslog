lib_LTLIBRARIES = libws.la

libws_la_SOURCES = \
	bitfield.c \
	conf.c \
	err.c \
	nybble.c \
	util.c \
	serial.c \
	ws23xx/decoder.c \
	ws23xx/ws23xx.c

libws_la_CFLAGS = \
	-D_GNU_SOURCES -I../../common -I..

libws_la_LIBADD = \
	-lm
