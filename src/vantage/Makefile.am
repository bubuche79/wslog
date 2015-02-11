include ../include.mk

bin_PROGRAMS = vantage

vantage_SOURCES = \
	main.c

vantage_CPPFLAGS = \
	$(inc_CPPFLAGS) -I..

vantage_CFLAGS = \
	$(inc_CFLAGS)

vantage_LDADD = \
	-L../libws -lws

dist_man_MANS = \
	vantage.1
