bin_PROGRAMS = wslogd

wslogd_SOURCES = \
	board.c \
	conf.c \
	daemon.c \
	sqlite.c \
	worker.c \
	wslogd.c \
	ws23xx.c \
	wunder.c

wslogd_CFLAGS = \
	-D_GNU_SOURCE \
	-I../../common -I../../lib

wslogd_LDADD = \
	$(INTLLIBS)
