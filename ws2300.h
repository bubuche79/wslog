#ifndef _WS2300_H
#define _WS2300_H

#include <unistd.h>
#include <stdint.h>

#include "convert.h"

#ifndef DEBUG
#define DEBUG 0
#endif

#define SETBIT			0x12
#define UNSETBIT		0x32
#define WRITENIB		0x42

enum ws_type
{
	WS_TEMP
};

struct ws_measure
{
	uint16_t addr;				/* nybble address */
	char id[8];					/* short name */
	enum ws_type type;			/* data type */
	char desc[64];				/* long name */
	char reset[8];				/* id of measure to reset this one */
};

inline uint8_t
nybble_at(const uint8_t *buf, size_t i)
{
	return (i & 0x1) ? buf[i / 2] >> 4 : buf[i / 2] & 0xF;

}

extern int ws_reset_06(int fd);
extern int ws_write_address(int fd, uint16_t address);

extern int ws_write_data(int fd, uint16_t address, size_t len, uint8_t encode_constant, const uint8_t *buf);
extern int ws_write_safe(int fd, uint16_t address, size_t len, uint8_t encode_constant, const uint8_t *buf);
extern int ws_read_data(int fd, uint16_t address, size_t len, uint8_t *buf);
extern int ws_read_safe(int fd, uint16_t address, size_t len, uint8_t *buf);
extern int ws_read_batch(int fd, const uint16_t *address, const size_t *len, size_t sz, uint8_t *buf);

#endif	/* ws2300.h */
