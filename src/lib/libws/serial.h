#ifndef _LIBWS_SERIAL_H
#define _LIBWS_SERIAL_H

#include <stdint.h>

extern long ws_io_delay;

#ifdef __cplusplus
extern "C" {
#endif

int ws_open(const char *device);
int ws_close(int fd);

int ws_read_byte(int fd, uint8_t *byte, long timeout);
int ws_write_byte(int fd, uint8_t byte);
int ws_clear(int fd);
int ws_flush(int fd);

#ifdef __cplusplus
}
#endif

#endif	/* _LIBWS_SERIAL_H */
