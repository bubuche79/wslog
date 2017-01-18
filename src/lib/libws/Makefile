SRC = ../..

LIBRARY = libws.a
VERS = .1
OBJECTS = \
	bitfield.o \
	conf.o \
	err.o \
	nybble.o \
	util.o \
	serial.o \
	ws23xx/archive.o \
	ws23xx/decoder.o \
	ws23xx/ws23xx.o

include	../Makefile.lib

LIBS = $(DYNLIB)

CPPFLAGS += -I$(SRC)/common -I.. -fPIC
LDLIBS += -lm

all: $(LIBS) $(LIBLINK)

install: all $(ROOTLIB) $(ROOTLIBS) $(ROOTLINKS)

include ../Makefile.targ
