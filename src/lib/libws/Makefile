SRC = ../..

LIBRARY = libws.a
VERS = .1
OBJECTS = \
	bitfield.o \
	nybble.o

include	../Makefile.lib

LIBS = $(DYNLIB)

CPPFLAGS += -I$(SRC)/common -I..
#LDLIBS += -lrt

all: $(LIBS) $(LIBLINK)

install: all $(ROOTLIB) $(ROOTLIBS) $(ROOTLINKS)

include ../Makefile.targ
