include ../include.mk

AM_CPPFLAGS = \
	$(inc_CPPFLAGS) -I../common -I..

AM_CFLAGS = \
	$(inc_CFLAGS)

lib_LTLIBRARIES = libwslog.la

libwslog_la_SOURCES = \
	board.c \
	conf.c \
	dataset.c \
	driver/driver.c \
	conf.h \
	board.h \
	dataset.h \
	driver/driver.h

libwslog_la_LIBADD = \
	-L../libws -lws -lrt -lpthread

if USE_WS23XX
libwslog_la_SOURCES += \
	driver/ws23xx.c \
	driver/ws23xx.h
endif

if USE_SIMU
libwslog_la_SOURCES += \
	driver/simu.c \
	driver/simu.h
endif

if USE_WSVIEW
libwslog_la_SOURCES += \
	wsview.c \
	wsview.h

libwslog_la_LIBADD += \
	-llua
endif

bin_PROGRAMS = wslogd wslogc

wslogc_SOURCES = \
	wslogc.c

wslogc_LDADD = \
	-L. -lwslog -lrt -lpthread

EXTRA_wslogc_DEPENDENCIES = \
	libwslog.la

wslogd_SOURCES = \
	daemon.c \
	db/sqlite.c \
	service/archive.c \
	service/sensor.c \
	service/util.c \
	worker.c \
	wslogd.c \
	wunder.c \
	board.h \
	daemon.h \
	db/sqlite.h \
	service/archive.h \
	service/sensor.h \
	service/util.h \
	worker.h \
	wslogd.h \
	wunder.h

wslogd_LDADD = \
	-L. -lwslog -lsqlite3 -lcurl -lrt -lpthread

EXTRA_wslogd_DEPENDENCIES = \
	libwslog.la

dist_sysconf_DATA = \
	wslogd.conf
