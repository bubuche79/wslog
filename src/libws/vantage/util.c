#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include "libws/serial.h"
#include "libws/crc_ccitt.h"
#include "libws/vantage/util.h"
#include "libws/vantage/vantage.h"

#define CMD_MAX		16	/* Maximum size of command */
#define ACK_MAX		6	/* Maximum size of ACK/OK response */

struct proc_def
{
	const char *fmt;
	int flags;
};

static const uint8_t ACK_TEST[] = { LF, CR, 'T', 'E', 'S', 'T', LF, CR };
static const uint8_t ACK_OK[] = { LF, CR, 'O', 'K', LF, CR };
static const uint8_t ACK_ACK[] = { ACK };
static const uint8_t ACK_DONE[] = { 'D', 'O', 'N', 'E', LF, CR };

static const struct proc_def CMDS[] =
{
	{ "TEST\n", IO_TEST },
	{ "WRD%c%c\n", IO_ACK },
	{ "RXCHECK\n", IO_OK },
	{ "RXTEST\n" },
	{ "VER\n", IO_OK },
	{ "RECEIVERS\n", IO_OK },
	{ "NVER\n", IO_OK },
	{ "LOOP %d\n", IO_ACK },
	{ "LPS %d %d\n", IO_ACK },
	{ "HILOWS\n", IO_ACK },
	{ "PUTRAIN %ld\n", IO_ACK },
	{ "PUTET %ld\n", IO_ACK },
	{ "DMP\n", IO_ACK },
	{ "DMPAFT\n", IO_ACK|IO_T10 },
	{ "GETEE\n", IO_ACK },
	{ "EERD %hX %lX\n", 0 },
	{ "EEWR %hX %hX\n", IO_OK },
	{ "EEBRD %hX %lX\n", IO_ACK },
	{ "EEBWR %hX %lX\n", IO_ACK },
	{ "CALED\n", IO_ACK },
	{ "CALFIX\n", IO_ACK },
	{ "BAR=%d %d\n", IO_OK },
	{ "BARDATA\n", IO_OK },
	{ "CLRLOG\n", IO_ACK|IO_T100 },
	{ "CLRALM\n", IO_OK_DONE|IO_T100 },
	{ "CLRCAL\n", IO_OK_DONE|IO_T100 },
	{ "CLRGRA\n", IO_OK_DONE|IO_T100 },
	{ "CLRVAR %d\n", IO_ACK|IO_T100 },
	{ "CLRHIGHS %d\n", IO_ACK|IO_T100 },
	{ "CLRLOWS %d\n", IO_ACK|IO_T100 },
	{ "CLRBITS\n", IO_ACK|IO_T100 },
	{ "CLRDATA\n", IO_ACK|IO_T100 },
	{ "BAUD %d\n", IO_OK },
	{ "SETTIME\n", IO_ACK },
	{ "GETTIME\n", IO_ACK },
	{ "GAIN %d\n", IO_OK },
	{ "SETPER %d\n", IO_ACK },
	{ "STOP\n" },
	{ "START\n" },
	{ "NEWSETUP\n", IO_ACK },
	{ "LAMPS %d\n", IO_OK },
};

const struct timespec IO_TIMEOUT = { .tv_sec = 0, .tv_nsec = 200000000 };

static void
vantage_to(int flags, struct timespec *ts)
{
	if (flags & IO_TMASK) {
		long tenth = (flags & IO_TMASK) >> 8;

		ts->tv_sec = tenth / 10;
		ts->tv_nsec = (tenth - 10 * ts->tv_sec) * 100000000;
	} else {
		ts->tv_sec = IO_TIMEOUT.tv_sec;
		ts->tv_nsec = IO_TIMEOUT.tv_nsec;
	}
}

static int
vantage_ack_ck(int fd, const uint8_t *ack, size_t acklen, const struct timespec *ts)
{
	char buf[acklen];

	if (vantage_read_to(fd, buf, acklen, ts) == -1) {
		goto error;
	}

	if (memcmp(ack, buf, acklen)) {
		errno = EIO;
		goto error;
	}

	return 0;

error:
	return -1;
}

/**
 * Read I/O acknowledge.
 *
 * The `ts' timeout argument is only used when `mode' is set to ACK_DONE or
 * ACK_ACK. The default timeout is used otherwise.
 */
static int
vantage_ack(int fd, int mode, const struct timespec *ts)
{
	int ret;

	switch (mode) {
	case IO_TEST:
		ret = vantage_ack_ck(fd, ACK_TEST, sizeof(ACK_TEST), &IO_TIMEOUT);
		break;
	case IO_OK:
		ret = vantage_ack_ck(fd, ACK_OK, sizeof(ACK_OK), &IO_TIMEOUT);
		break;
	case IO_OK_DONE:
		if (vantage_ack_ck(fd, ACK_OK, sizeof(ACK_OK), &IO_TIMEOUT) == -1) {
			goto error;
		}
		ret = vantage_ack_ck(fd, ACK_DONE, sizeof(ACK_DONE), ts);
		break;
	case IO_ACK:
		ret = vantage_ack_ck(fd, ACK_ACK, sizeof(ACK_ACK), ts);
		break;
	default:
		errno = EINVAL;
		ret = -1;
		break;
	}

	return ret;

error:
	return -1;
}

/**
 * Vantage I/O read.
 *
 * The `ts' timeout only applies to the first I/O wait. Upon first read, the
 * timeout is set to IO_TIMEOUT (0.2 second).
 */
ssize_t
vantage_read_to(int fd, void *buf, size_t len, const struct timespec *ts)
{
	ssize_t sz;

	sz = 0;

	while (sz < len) {
		ssize_t ret;

		if ((ret = ws_read_to(fd, buf + sz, len - sz, ts)) == -1) {
			goto error;
		} else if (ret == 0) {
			errno = ETIME;
			goto error;
		}

		sz += ret;
		ts = &IO_TIMEOUT;
	}

	return sz;

error:
	return -1;
}

ssize_t
vantage_read(int fd, void *buf, size_t len)
{
	return vantage_read_to(fd, buf, len, &IO_TIMEOUT);
}

ssize_t
vantage_write(int fd, const void *buf, size_t len)
{
	return ws_write(fd, buf, len);
}

int
vantage_pread(int fd, int flags, void *buf, size_t len)
{
	if (flags & IO_ACK_MASK) {
		errno = EINVAL;
		goto error;
	}

	if (vantage_read(fd, buf, len) == -1) {
		goto error;
	}

	if (flags & IO_CRC) {
		uint16_t crc;
		uint8_t crcbuf[2];

		if (vantage_read(fd, crcbuf, sizeof(crcbuf)) == -1) {
			goto error;
		}

		crc = ws_crc_ccitt(0, buf, len);
		crc = ws_crc_ccitt(crc, crcbuf, sizeof(crcbuf));

		if (crc != 0) {
			errno = -1;
			goto error;
		}
	}

	return 0;

error:
	return -1;
}

int
vantage_pwrite(int fd, int flags, const void *buf, size_t len)
{
	uint8_t crc[2];
	struct iovec iov[2];
	size_t iovcnt;

	/* I/O */
	iovcnt = 1;
	iov[0].iov_base = (void *) buf;
	iov[0].iov_len = len;

	if (flags & IO_CRC) {
		uint16_t v;

		iovcnt++;
		iov[1].iov_base = crc;
		iov[1].iov_len = sizeof(crc);

		/* Compute CRC */
		v = ws_crc_ccitt(0, buf, len);

		crc[0] = v >> 8;
		crc[1] = v & 0xFF;
	}

	if (ws_writev(fd, iov, iovcnt) == -1) {
		goto error;
	}

	/* Acknowledge */
	if (flags & IO_ACK_MASK) {
		struct timespec ts;

		vantage_to(flags, &ts);

		if (vantage_ack(fd, flags & IO_ACK_MASK, &ts) == -1) {
			goto error;
		}
	}

	return 0;

error:
	return -1;
}

int
vantage_proc(int fd, enum vantage_cmd cmd, /* args */ ...)
{
	va_list ap;
	char buf[CMD_MAX];
	int bufsz;

	if (cmd < 0 && LAMPS < cmd) {
		errno = EINVAL;
		goto error;
	}

	va_start(ap, cmd);
	bufsz = vsnprintf(buf, sizeof(buf), CMDS[cmd].fmt, ap);
	va_end(ap);

	return vantage_pwrite(fd, CMDS[cmd].flags, buf, bufsz);

error:
	return -1;
}
