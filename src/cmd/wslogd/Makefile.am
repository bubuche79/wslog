bin_PROGRAMS = wslogd

wslogd_SOURCES = \
	board.c \
	conf.c \
	daemon.c \
	sqlite.c \
	main.c \
	worker.c \
	wunder.c \
	board.h \
	conf.h \
	daemon.h \
	sqlite.h \
	worker.h \
	wslogd.h \
	wunder.h

if HAVE_WS23XX
wslogd_SOURCES += \
	ws23xx.c \
	ws23xx.h
endif

wslogd_CFLAGS = \
	-I../../common -I../../lib

wslogd_LDADD = \
	-L../../lib/libws -lws -lsqlite3 -lcurl -lrt -lpthread

dist_sysconf_DATA = \
	wslogd.conf
