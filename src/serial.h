#ifndef _SERIAL_H
#define _SERIAL_H

#include <stdint.h>

extern long ws_io_delay;

int ws_open(const char *device);
int ws_close(int fd);

int ws_read_byte(int fd, uint8_t *byte, long timeout);
int ws_write_byte(int fd, uint8_t byte);
int ws_clear(int fd);
int ws_flush(int fd);

#endif	/* _SERIAL_H */
