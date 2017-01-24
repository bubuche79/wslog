bin_PROGRAMS = ws23xx

ws23xx_SOURCES = \
	main.c \
	util.c \
	util.h

ws23xx_CFLAGS = \
	-DDEBUG -std=c11 -pedantic -D_XOPEN_SOURCE=700 -Wall -Werror -I../../common -I../../lib

ws23xx_LDADD = \
	-L../../lib/libws -lws
