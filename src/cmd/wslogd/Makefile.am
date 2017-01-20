bin_PROGRAMS = wslogd

wslogd_SOURCES = \
	board.c \
	conf.c \
	daemon.c \
	service/sensor.c \
	sqlite.c \
	main.c \
	worker.c \
	wunder.c \
	board.h \
	conf.h \
	daemon.h \
	service/sensor.h \
	sqlite.h \
	worker.h \
	wslogd.h \
	wunder.h

if HAVE_WS23XX
wslogd_SOURCES += \
	driver/ws23xx.c \
	driver/ws23xx.h
endif

wslogd_CFLAGS = \
	-DDEBUG -Wall -Werror -I../../common -I../../lib

wslogd_LDADD = \
	-L../../lib/libws -lws -lsqlite3 -lcurl -lrt -lpthread

dist_sysconf_DATA = \
	wslogd.conf
