#ifndef _LIBWS23XX_WS23XX_H
#define _LIBWS23XX_WS23XX_H

#include <unistd.h>
#include <stdint.h>

#define SETBIT			0x12
#define UNSETBIT		0x32
#define WRITENIB		0x42

#define WS23XX_WVAL_OK			0
#define WS23XX_WVAL_INVAL		1
#define WS23XX_WVAL_OVERFLOW	2

#ifdef __cplusplus
extern "C" {
#endif

int ws23xx_open();

int ws23xx_reset_06(int fd);
int ws23xx_write_addr(int fd, uint16_t addr);

int ws23xx_write(int fd, uint16_t addr, size_t len, uint8_t op, const uint8_t *buf);
int ws23xx_write_safe(int fd, uint16_t addr, size_t len, uint8_t op, const uint8_t *buf);
int ws23xx_read(int fd, uint16_t addr, size_t nnyb, uint8_t *buf);
int ws23xx_read_safe(int fd, uint16_t addr, size_t nnyb, uint8_t *buf);
int ws23xx_read_batch(int fd, const uint16_t *addr, const size_t *nnyb, size_t nel, uint8_t *buf[]);

#ifdef __cplusplus
}
#endif

#endif	/* _LIBWS23XX_WS23XX_H */
