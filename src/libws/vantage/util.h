#ifndef _LIBWS_VANTAGE_UTIL_H
#define _LIBWS_VANTAGE_UTIL_H

#include <sys/types.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int vantage_ack(int fd, const char *cmd, size_t cmdlen, void *buf, size_t len);
int vantage_ack_crc(int fd, const char *cmd, size_t cmdlen, void *buf, size_t len);

int vantage_ok(int fd, const char *cmd, size_t cmdlen, void *buf, size_t len);

int vantage_write_crc(int fd, const uint8_t *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif	/* _LIBWS_VANTAGE_UTIL_H */
