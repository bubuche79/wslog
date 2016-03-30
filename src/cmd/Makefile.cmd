include		$(SRC)/Makefile.master

#
# Command paths
#
FILEMODE =	755

#
# Default paths or names
#
ROOTBIN =	$(ROOT)/bin
ROOTETC =	$(ROOT)/etc
ROOTMAN =	$(ROOT)/share/man
ROOTSBIN =	$(ROOT)/sbin
ROOTMAN1 =	$(ROOTMAN)/man1
ROOTMAN5 =	$(ROOTMAN)/man5

ROOTPROG =	$(PROG:%=$(ROOTBIN)/%)
ROOTSBINPROG =	$(PROG:%=$(ROOTSBIN)/%)

ROOTETCFILES =	$(ETCFILES:%=$(ROOTETC)/%)
ROOTMAN1FILES =	$(MAN1FILES:%=$(ROOTMAN1)/%)
ROOTMAN5FILES =	$(MAN5FILES:%=$(ROOTMAN5)/%)

$(ROOTETCFILES): FILEMODE = 644
$(ROOTMAN1FILES): FILEMODE = 644
$(ROOTMAN5FILES): FILEMODE = 644

#
# Link flags
#
LDLIBS =

SRCS =		$(OBJECTS:%.o=%.c)
OBJS =		$(OBJECTS:%=objs/%)
DEPS =		$(OBJS:%.o=%.d)

#
# Compilation rules
#
objs/%.o: %.c
	$(COMPILE.c) -o $@ $<
	$(POST_PROCESS.o)

.PRECIOUS:	$(PROG)
