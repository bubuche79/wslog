include ../include.mk

bin_PROGRAMS = vantage

vantage_SOURCES = \
	main.c

vantage_CPPFLAGS = \
	$(inc_CPPFLAGS) -I../common -I..

vantage_CFLAGS = \
	$(inc_CFLAGS)

vantage_LDADD = \
	-L../libws -lws
