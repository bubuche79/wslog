include ../include.mk

AM_CPPFLAGS = \
	$(inc_CPPFLAGS)

AM_CFLAGS = \
	$(inc_CFLAGS)

bin_PROGRAMS = wslog

wslog_SOURCES = \
	ic-omm.c \
	wslog.c

wslog_CPPFLAGS = \
	$(inc_CPPFLAGS) -I..

wslog_CFLAGS = \
	$(inc_CFLAGS) \
	@SQLITE3_CFLAGS@

wslog_LDADD = \
	-L../libws -lws \
	@SQLITE3_LIBS@
