#ifndef _WS2300_H
#define _WS2300_H

#include <unistd.h>
#include <stdint.h>

#include "decoder.h"

#ifndef DEBUG
#define DEBUG 0
#endif

#define SETBIT			0x12
#define UNSETBIT		0x32
#define WRITENIB		0x42

struct ws_measure
{
	uint16_t addr;				/* nybble addr */
	char id[8];					/* short name */
	enum ws_type type;			/* data converter type */
	char desc[64];				/* long name */
	char reset[8];				/* id of measure to reset this one */
};

extern int ws_reset_06(int fd);
extern int ws_write_addr(int fd, uint16_t addr);

extern int ws_write_data(int fd, uint16_t addr, size_t len, uint8_t encode_constant, const uint8_t *buf);
extern int ws_write_safe(int fd, uint16_t addr, size_t len, uint8_t encode_constant, const uint8_t *buf);
extern int ws_read_data(int fd, uint16_t addr, size_t nybble, uint8_t *buf);
extern int ws_read_safe(int fd, uint16_t addr, size_t nybble, uint8_t *buf);
extern int ws_read_batch(int fd, const uint16_t *addr, const size_t *nybble, size_t sz, uint8_t *buf[]);

#endif	/* ws2300.h */
