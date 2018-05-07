include ../include.mk

AM_CPPFLAGS = \
	$(inc_CPPFLAGS) -I../common -I.. -I../wslogd

AM_CFLAGS = \
	$(inc_CFLAGS)

lib_LTLIBRARIES = wsview.la

wsview_la_SOURCES = \
	../wslogd/board.c \
	../wslogd/dataset.c \
	../wslogd/board.h \
	../wslogd/dataset.h \
	wsview.c \
	wsview.h

wsview_la_LDFLAGS = \
	-module

wsview_la_LIBADD = \
	-L../libws -lws -llua -lsqlite3 -lrt -lpthread

cluadir = $(libdir)/lua/5.3
luadir = $(datadir)/lua/5.3
wwwdir = $(localstatedir)/www

nobase_dist_lua_DATA = \
	wsview/*.lua \
	wsview/sgi/*.lua

nobase_dist_www_DATA = \
	cgi-bin/* \
	wsview-static/*.*
