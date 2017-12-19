#ifndef _LIBWS_SERIAL_H
#define _LIBWS_SERIAL_H

#include <termios.h>
#include <sys/types.h>

extern long ws_io_delay;

#ifdef __cplusplus
extern "C" {
#endif

int ws_open(const char *device, speed_t speed);
int ws_close(int fd);

ssize_t ws_read(int fd, void *buf, size_t len);
ssize_t ws_read_to(int fd, void *buf, size_t len, long timeout);
ssize_t ws_write(int fd, const void *buf, size_t len);
int ws_flush(int fd);

#ifdef __cplusplus
}
#endif

#endif	/* _LIBWS_SERIAL_H */
