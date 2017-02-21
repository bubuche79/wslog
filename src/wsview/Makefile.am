include ../include.mk

AM_CPPFLAGS = \
	$(inc_CPPFLAGS) -I../common -I.. -I../wslogd

AM_CFLAGS = \
	$(inc_CFLAGS)

lib_LTLIBRARIES = libwsview.la

libwsview_la_SOURCES = \
	../wslogd/board.c \
	../wslogd/dataset.c \
	../wslogd/board.h \
	../wslogd/dataset.h \
	wsview.c \
	wsview.h

libwsview_la_LIBADD = \
	-L../libws -lws -llua -lrt -lpthread

dist_data_DATA = \
	cgi-bin/* \
	wsview/* \
	wsview-static/*

deploy:
	rsync -a cgi-bin wsview-static root@lede1701:/www
	rsync -a wsview root@lede1701:/usr/lib/lua
