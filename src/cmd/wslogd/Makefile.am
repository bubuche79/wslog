bin_PROGRAMS = wslogd

wslogd_SOURCES = \
	board.c \
	conf.c \
	daemon.c \
	sqlite.c \
	main.c \
	worker.c \
	ws23xx.c \
	wunder.c \
	board.h \
	conf.h \
	daemon.h \
	sqlite.h \
	worker.h \
	wslogd.h \
	ws23xx.h \
	wunder.h

wslogd_CFLAGS = \
	-I../../common -I../../lib

wslogd_LDADD = \
	-L../../lib/libws -lws -lsqlite3 -lcurl -lrt -lpthread

dist_sysconf_DATA = \
	wslogd.conf
