include ../include.mk

lib_LTLIBRARIES = libws.la

libws_la_SOURCES = \
	aggregate.c \
	aggregate.h \
	cli.c \
	cli.h \
	conf.c \
	conf.h \
	crc_ccitt.c \
	crc_ccitt.h \
	nybble.c \
	nybble.h \
	util.c \
	util.h \
	serial.c \
	defs.h

nobase_pkginclude_HEADERS = \
	serial.h

if USE_VANTAGE
libws_la_SOURCES += \
	vantage/clearing.c \
	vantage/config.c \
	vantage/current.c \
	vantage/download.c \
	vantage/eeprom.c \
	vantage/serial.c \
	vantage/testing.c \
	vantage/unit.c \
	vantage/util.c \
	vantage/util.h \
	vantage/vantage.h

nobase_pkginclude_HEADERS += \
	vantage/vantage.h
endif

if USE_WS23XX
libws_la_SOURCES += \
	ws23xx/archive.c \
	ws23xx/archive.h \
	ws23xx/decoder.c \
	ws23xx/decoder.h \
	ws23xx/ws23xx.c \
	ws23xx/ws23xx.h

nobase_pkginclude_HEADERS += \
	ws23xx/ws23xx.h
endif

libws_la_CPPFLAGS = \
	$(inc_CPPFLAGS) -I..

libws_la_CFLAGS = \
	$(inc_CFLAGS)

libws_la_LIBADD = \
	-lm
