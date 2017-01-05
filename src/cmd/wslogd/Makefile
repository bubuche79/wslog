SRC =		../..

PROG =		wslogd
OBJECTS = \
	conf.o \
	daemon.o \
	board.o \
	worker.o \
	wslogd.o

include ../Makefile.cmd

CPPFLAGS +=	-DHAVE_SELECT \
		-I$(SRC)/common -I$(SRC)/lib

LDLIBS +=	-L$(SRC)/lib/libws -lws -lcurl

all:		$(PROG)

install:	all $(ROOTBIN) $(ROOTPROG)

include ../Makefile.targ
