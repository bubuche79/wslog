#ifndef _SERIAL_H
#define _SERIAL_H

#include <stdint.h>

extern int ws_open(const char *device);
extern int ws_close(int fd);

extern int ws_read_byte(int fd, uint8_t *byte, long timeout);
extern int ws_write_byte(int fd, uint8_t byte);
extern int ws_clear(int fd);
extern int ws_flush(int fd);

#endif	/* _SERIAL_H */
