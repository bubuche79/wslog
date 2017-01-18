SRC =		../..

PROG =		ws23xx
OBJECTS = \
	main.o

include ../Makefile.cmd

CPPFLAGS +=	-DHAVE_SELECT \
		-I$(SRC)/common -I$(SRC)/lib

LDLIBS +=	-L$(SRC)/lib/libws -lws

all:		$(PROG)

install:	all $(ROOTBIN) $(ROOTPROG)

include ../Makefile.targ
