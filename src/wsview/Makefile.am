include ../include.mk

cluadir = $(libdir)/lua/5.3
luadir = $(datadir)/lua/5.3
wwwdir = $(localstatedir)/www

AM_CPPFLAGS = \
	$(inc_CPPFLAGS) -I.. -I../wslogd

AM_CFLAGS = \
	$(inc_CFLAGS)

pkglibdir = $(cluadir)
pkglib_LTLIBRARIES = wsview.la

wsview_la_SOURCES = \
	../wslogd/board.c \
	../wslogd/dataset.c \
	../wslogd/board.h \
	../wslogd/dataset.h \
	wsview.c \
	wsview.h

wsview_la_LDFLAGS = \
	-module -avoid-version

wsview_la_LIBADD = \
	-L../libws -lws -llua -lsqlite3 -lrt -lpthread

nobase_dist_lua_DATA = \
	wsview/*.lua \
	wsview/sgi/*.lua

nobase_dist_www_DATA = \
	wsview-static/*.*

nobase_dist_www_SCRIPTS = \
	cgi-bin/*
