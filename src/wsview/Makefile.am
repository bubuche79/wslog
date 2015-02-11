include ../include.mk

LUA_MAJOR = 5.3

cluadir = $(libdir)/lua/$(LUA_MAJOR)
luadir = $(datadir)/lua/$(LUA_MAJOR)
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

wsview_la_CFLAGS = \
	@LUA_CFLAGS@ @SQLITE3_CFLAGS@

wsview_la_LDFLAGS = \
	-module -avoid-version

wsview_la_LIBADD = \
	-L../libws -lws \
	@LUA_LIBS@ @SQLITE3_LIBS@ \
	-lm -lrt -lpthread 

nobase_dist_lua_DATA = \
	wsview/*.lua \
	wsview/rest/*.lua \
	wsview/sgi/*.lua

nobase_dist_www_DATA = \
	wsview-static/*.* \
	wsview-static/bootstrap/*.*

nobase_dist_www_SCRIPTS = \
	cgi-bin/*
