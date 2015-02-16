#ifndef _WS2300_H
#define _WS2300_H

#include <stdint.h>

#ifndef DEBUG
#define DEBUG 0
#endif

enum type {
	DATE,
	TEMP
};

struct measure {
	uint16_t address;
	char id[8];
	enum type type;
	char desc[64];
};

extern int ws_reset_06(int fd);
extern int ws_write_address(int fd, uint16_t address);

extern int ws_write_data(int fd, uint16_t address, size_t len, uint8_t encode_constant, const uint8_t *buf);
extern int ws_write_safe(int fd, uint16_t address, size_t len, uint8_t encode_constant, const uint8_t *buf);
extern int ws_read_data(int fd, uint16_t address, size_t len, uint8_t *buf);
extern int ws_read_safe(int fd, uint16_t address, size_t len, uint8_t *buf);
extern int ws_read_batch(int fd, const uint16_t *address, const size_t *len, size_t sz, uint8_t *buf);

#endif	/* ws2300.h */
