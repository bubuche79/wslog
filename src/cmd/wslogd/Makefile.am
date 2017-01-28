bin_PROGRAMS = wslogd wslogc

wslogc_SOURCES = \
	board.c \
	wslogc.c \
	board.h

wslogc_CPPFLAGS = \
	-DDEBUG -std=c11 -pedantic -D_XOPEN_SOURCE=700 -Wall -Werror -I../../common -I../../lib

wslogc_LDADD = \
	-L../../lib/libws -lws -lrt -lpthread

wslogd_SOURCES = \
	board.c \
	conf.c \
	daemon.c \
	db/sqlite.c \
	driver/driver.c \
	service/archive.c \
	service/sensor.c \
	service/util.c \
	main.c \
	worker.c \
	wunder.c \
	board.h \
	conf.h \
	daemon.h \
	db/sqlite.h \
	driver/driver.h \
	service/archive.h \
	service/sensor.h \
	service/util.h \
	worker.h \
	wslogd.h \
	wunder.h

if HAVE_WS23XX
wslogd_SOURCES += \
	driver/ws23xx.c \
	driver/ws23xx.h
endif

if HAVE_SIMU
wslogd_SOURCES += \
	driver/simu.c \
	driver/simu.h
endif

wslogd_CPPFLAGS = \
	-DDEBUG -std=c11 -pedantic -D_XOPEN_SOURCE=700 -Wall -Werror -I../../common -I../../lib

wslogd_LDADD = \
	-L../../lib/libws -lws -lsqlite3 -lcurl -lrt -lpthread

dist_sysconf_DATA = \
	wslogd.conf
