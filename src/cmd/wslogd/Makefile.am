noinst_LIBRARIES = libwslog.a

libwslog_a_SOURCES = \
	board.c \
	conf.c \
	driver/driver.c \
	conf.h \
	board.h \
	driver/driver.h

if HAVE_WS23XX
libwslog_a_SOURCES += \
	driver/ws23xx.c \
	driver/ws23xx.h
endif

if HAVE_SIMU
libwslog_a_SOURCES += \
	driver/simu.c \
	driver/simu.h
endif

libwslog_a_CPPFLAGS = \
	-DDEBUG -std=c11 -pedantic -D_XOPEN_SOURCE=700 -Wall -Werror -I../../common -I../../lib

bin_PROGRAMS = wslogd wslogc

wslogc_SOURCES = \
	wslogc.c

wslogc_CPPFLAGS = \
	-DDEBUG -std=c11 -pedantic -D_XOPEN_SOURCE=700 -Wall -Werror -I../../common -I../../lib

wslogc_LDADD = \
	-L. -lwslog -L../../lib/libws -lws -lrt -lpthread

wslogd_SOURCES = \
	daemon.c \
	dataset.c \
	db/sqlite.c \
	service/archive.c \
	service/sensor.c \
	service/util.c \
	main.c \
	worker.c \
	wunder.c \
	board.h \
	daemon.h \
	dataset.h \
	db/sqlite.h \
	service/archive.h \
	service/sensor.h \
	service/util.h \
	worker.h \
	wslogd.h \
	wunder.h

wslogd_CPPFLAGS = \
	-DDEBUG -std=c11 -pedantic -D_XOPEN_SOURCE=700 -Wall -Werror -I../../common -I../../lib

wslogd_LDADD = \
	-L. -lwslog -L../../lib/libws -lws -lsqlite3 -lcurl -lrt -lpthread

dist_sysconf_DATA = \
	wslogd.conf
