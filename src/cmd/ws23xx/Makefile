SRC =		../..

PROG =		ws23xx
OBJECTS = \
	decoder.o \
	history.o \
	main.o \
	serial.o \
	ws2300.o

include ../Makefile.cmd

CPPFLAGS +=	-DHAVE_SELECT \
		-I$(SRC)/common -I$(SRC)/lib

LDLIBS +=	-L$(SRC)/lib/libws -lws \
		-lcurl -lm

all:		$(PROG)

install:	all $(ROOTBIN) $(ROOTPROG)

include ../Makefile.targ
