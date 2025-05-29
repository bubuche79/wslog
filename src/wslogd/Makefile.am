include ../include.mk

bin_PROGRAMS = wslogd

wslogd_SOURCES = \
	board.c \
	board.h \
	conf.c \
	conf.h \
	curl.c \
	curl.h \
	dataset.c \
	dataset.h \
	db/sqlite.c \
	db/sqlite.h \
	driver/driver.c \
	driver/driver.h \
	service/archive.c \
	service/archive.h \
	service/ic.c \
	service/ic.h \
	service/sensor.c \
	service/sensor.h \
	service/sync.c \
	service/sync.h \
	service/util.c \
	service/util.h \
	service/wunder.c \
	service/wunder.h \
	worker.c \
	worker.h \
	wslogd.c \
	wslogd.h

if USE_VANTAGE
wslogd_SOURCES += \
	driver/vantage.c \
	driver/vantage.h
endif

if USE_WS23XX
wslogd_SOURCES += \
	driver/ws23xx.c \
	driver/ws23xx.h
endif

if USE_VIRT
wslogd_SOURCES += \
	driver/virt.c \
	driver/virt.h
endif

wslogd_CPPFLAGS = \
	$(inc_CPPFLAGS) -I..

wslogd_CFLAGS = \
	$(inc_CFLAGS) \
	@SQLITE3_CFLAGS@ @LIBCURL_CFLAGS@

wslogd_LDADD = \
	-L../libws -lws \
	@SQLITE3_LIBS@ @LIBCURL_LIBS@ \
	-lm -lrt -lpthread

dist_man_MANS = \
	wslogd.1 \
	wslogd.conf.5

dist_sysconf_DATA = \
	wslogd.conf

dist_wslogdata_DATA = \
	sqlite.sql
