SRC =		../..

PROG =		wslogd
OBJECTS = \
	board.o \
	conf.o \
	csv.o \
	daemon.o \
	worker.o \
	wslogd.o \
	ws23xx.o \
	wunder.o

include ../Makefile.cmd

CPPFLAGS +=	-DHAVE_SELECT \
		-D_USE_FORK \
		-I$(SRC)/common -I$(SRC)/lib

LDLIBS +=	-L$(SRC)/lib/libws -lws -lcurl -lpthread -lrt

all:		$(PROG)

install:	all $(ROOTBIN) $(ROOTPROG)

include ../Makefile.targ
