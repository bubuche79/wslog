SRC =		../..

PROG =		wslogd
OBJECTS = \
	board.o \
	conf.o \
	daemon.o \
	sqlite.o \
	worker.o \
	wslogd.o \
	ws23xx\ws23xx.o \
	wunder.o

include ../Makefile.cmd

CPPFLAGS +=	-DHAVE_SELECT \
		-D_GNU_SOURCE \
		-I$(SRC)/common -I$(SRC)/lib

LDLIBS +=	-L$(SRC)/lib/libws -lws -lcurl -lsqlite3 -lpthread -lrt

all:		$(PROG)

install:	all $(ROOTBIN) $(ROOTPROG)

include ../Makefile.targ
