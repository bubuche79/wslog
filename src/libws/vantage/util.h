#ifndef _LIBWS_VANTAGE_UTIL_H
#define _LIBWS_VANTAGE_UTIL_H

#include <sys/types.h>
#include <sys/time.h>
#include <stdint.h>

#include "libws/vantage/vantage.h"

/* I/O mode */
#define IO_CRC		0x0001
#define IO_LONG_ACK	0x0002

/* Acknowledge mode */
#define IO_TEST		0x0010
#define IO_OK		0x0020
#define IO_OK_DONE	0x0030
#define IO_ACK		0x0040
#define IO_ACK_MASK	0x00F0

/* I/O timeouts */
#define IO_TIMEOUT	250		/* I/O timeout, in milliseconds */
#define IO_LONG_TIMEOUT	60000		/* Long I/O timeout (ack), in milliseconds */

enum vantage_cmd
{
	TEST = 0,
	WRD,
	RXCHECK,
	RXTEST,
	VER,
	RECEIVERS,
	NVER,
	LOOP,
	LPS,
	HILOWS,
	PUTRAIN,
	PUTET,
	DMP,
	DMPAFT,
	GETEE,
	EERD,
	EEWR,
	EEBRD,
	EEBWR,
	CALED,
	CALFIX,
	BAR,
	BARDATA,
	CLRLOG,
	CLRALM,
	CLRCAL,
	CLRGRA,
	CLRVAR,
	CLRHIGHS,
	CLRLOWS,
	CLRBITS,
	CLRDATA,
	BAUD,
	SETTIME,
	GETTIME,
	GAIN,
	SETPER,
	STOP,
	START,
	NEWSETUP,
	LAMPS
};

#ifdef __cplusplus
extern "C" {
#endif

double vantage_float(int v, int scale);

double vantage_temp(int f, int scale);
double vantage_pressure(int inhg, int scale);
double vantage_rain(int ticks, int rain_cup);
double vantage_speed(int mph);
const char *vantage_dir(int idx);

const char *vantage_type_str(enum vantage_type t);

int8_t vantage_int8(const uint8_t *buf, uint16_t off);
uint8_t vantage_uint8(const uint8_t *buf, uint16_t off);

int16_t vantage_int16(const uint8_t *buf, uint16_t off);
uint16_t vantage_uint16(const uint8_t *buf, uint16_t off);

time_t vantage_mktime(const uint8_t *buf, uint16_t off, int time);
void vantage_localtime(uint8_t *buf, time_t time);

ssize_t vantage_read(int fd, void *buf, size_t len);
ssize_t vantage_write(int fd, const void *buf, size_t len);

int vantage_pread(int fd, int flags, void *buf, size_t len);
int vantage_pread_to(int fd, int flags, void *buf, size_t len, long timeout);
int vantage_pwrite(int fd, int flags, const void *buf, size_t len);

int vantage_proc(int fd, enum vantage_cmd cmd, /* args */ ...);

#ifdef __cplusplus
}
#endif

#endif	/* _LIBWS_VANTAGE_UTIL_H */
