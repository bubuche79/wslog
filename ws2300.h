#ifndef _WS2300_H
#define _WS2300_H

#ifndef DEBUG
#define DEBUG 0
#endif

enum type {
	DATE,
	TEMP
};

struct measure {
	short address;
	char id[8];
	enum type type;
	char desc[64];
};

extern int ws_reset_06(int fd);

#endif	/* ws2300.h */
