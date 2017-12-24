#ifndef _LIBWS_VANTAGE_UTIL_H
#define _LIBWS_VANTAGE_UTIL_H

#include <sys/types.h>
#include <sys/time.h>
#include <stdint.h>

/* IO mode */
#define IO_CRC		0x0001

/* Acknowledge mode */
#define IO_ACK		0x0010
#define IO_OK		0x0020
#define IO_TEST		0x0030
#define IO_OK_DONE	0x0040
#define IO_ACK_MASK	0x00F0

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

time_t vantage_mktime(const uint8_t *buf);
void vantage_time(uint8_t *buf, time_t time);

ssize_t vantage_read(int fd, void *buf, size_t len);
ssize_t vantage_write(int fd, const void *buf, size_t len);

int vantage_pread(int fd, int flags, void *buf, size_t len);
int vantage_pwrite(int fd, int flags, const void *buf, size_t len);

int vantage_proc(int fd, enum vantage_cmd cmd, /* args */ ...);

#ifdef __cplusplus
}
#endif

#endif	/* _LIBWS_VANTAGE_UTIL_H */
