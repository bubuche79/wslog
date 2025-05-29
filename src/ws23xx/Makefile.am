include ../include.mk

bin_PROGRAMS = ws23xx

ws23xx_SOURCES = \
	util.c \
	util.h \
	ws23xx.c

ws23xx_CPPFLAGS = \
	$(inc_CPPFLAGS) -I..

ws23xx_CFLAGS = \
	$(inc_CFLAGS)

ws23xx_LDADD = \
	-L../libws -lws
