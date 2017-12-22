#ifndef _LIBWS_VANTAGE_UTIL_H
#define _LIBWS_VANTAGE_UTIL_H

#include <sys/types.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

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

int vantage_proc(int fd, enum vantage_cmd cmd, /* args */ ...);

int vantage_ack(int fd, const char *cmd, size_t cmdlen, void *buf, size_t len);
int vantage_ack_crc(int fd, const char *cmd, size_t cmdlen, void *buf, size_t len);

int vantage_ok(int fd, const char *cmd, size_t cmdlen, void *buf, size_t len);

int vantage_write_crc(int fd, const uint8_t *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif	/* _LIBWS_VANTAGE_UTIL_H */
