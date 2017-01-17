SRC =		../..

PROG =		ws23xx
OBJECTS = \
	history.o \
	main.o

include ../Makefile.cmd

CPPFLAGS +=	-DHAVE_SELECT \
		-I$(SRC)/common -I$(SRC)/lib

LDLIBS +=	-L$(SRC)/lib/libws -lws -lm

all:		$(PROG)

install:	all $(ROOTBIN) $(ROOTPROG)

include ../Makefile.targ
