include ../include.mk

AM_CPPFLAGS = \
	$(inc_CPPFLAGS) -I..

AM_CFLAGS = \
	$(inc_CFLAGS)

noinst_LIBRARIES = libwslog.a

libwslog_a_SOURCES = \
	board.c \
	conf.c \
	curl.c \
	driver/driver.c \
	service/util.c \
	board.h \
	conf.h \
	curl.h \
	driver/driver.h \
	service/util.c

if USE_VANTAGE
libwslog_a_SOURCES += \
	driver/vantage.c \
	driver/vantage.h
endif

if USE_WS23XX
libwslog_a_SOURCES += \
	driver/ws23xx.c \
	driver/ws23xx.h
endif

if USE_VIRT
libwslog_a_SOURCES += \
	driver/virt.c \
	driver/virt.h
endif

bin_PROGRAMS = wslogd wslogc

wslogc_SOURCES = \
	wslogc.c

wslogc_LDADD = \
	-L. -lwslog -L../libws -lws \
	-lm -lrt -lpthread

EXTRA_wslogc_DEPENDENCIES = \
	libwslog.a

wslogd_SOURCES = \
	dataset.c \
	db/sqlite.c \
	service/archive.c \
	service/ic.c \
	service/sensor.c \
	service/sync.c \
	service/util.c \
	service/wunder.c \
	worker.c \
	wslogd.c \
	board.h \
	dataset.h \
	db/sqlite.h \
	service/archive.h \
	service/ic.h \
	service/sensor.h \
	service/sync.h \
	service/util.h \
	service/wunder.h \
	worker.h \
	wslogd.h

wslogd_CFLAGS = \
	@SQLITE3_CFLAGS@ @LIBCURL_CFLAGS@

wslogd_LDADD = \
	-L. -lwslog -L../libws -lws \
	@SQLITE3_LIBS@ @LIBCURL_LIBS@ \
	-lm -lrt -lpthread

EXTRA_wslogd_DEPENDENCIES = \
	libwslog.a

dist_man_MANS = \
	wslogd.1 \
	wslogd.conf.5

dist_sysconf_DATA = \
	wslogd.conf

dist_wslogdata_DATA = \
	sqlite.sql
