#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include "libws/serial.h"
#include "libws/vantage/util.h"
#include "libws/vantage/vantage.h"

#define CMD_MAX		16	/* Maximum size of command */
#define ACK_MAX		6	/* Maximum size of ACK/OK response */

#define IO_TIMEOUT	250	/* Read timeout, in milliseconds */
#define IO_DONE_TIMEOUT	2000	/* DONE acknowledge timeout, in milliseconds */

enum ack_type
{
	AT_ACK,
	AT_OK,
	AT_TEST,
	AT_DONE,		/* OK and DONE after a while */
	AT_UNKOWN
};

struct proc_def
{
	const char *fmt;
	enum ack_type ack;
};

static const uint8_t DELIM[] = { LF, CR };

static const uint8_t ACK_TEST[] = { 'T', 'E', 'S', 'T', LF };
static const uint8_t ACK_OK[] = { LF, CR, 'O', 'K', LF, CR };
static const uint8_t ACK_ACK[] = { ACK };
static const uint8_t ACK_DONE[] = { 'D', 'O', 'N', 'E', LF, CR };

static const struct proc_def CMDS[] =
{
	{ "TEST\n", AT_TEST },
	{ "WRD%c%c\n", AT_ACK },
	{ "RXCHECK\n", AT_OK },
	{ "RXTEST\n", AT_UNKOWN },
	{ "VER\n", AT_OK },
	{ "RECEIVERS\n", AT_OK },
	{ "NVER\n", AT_OK },
	{ "LOOP %d\n", AT_ACK },
	{ "LPS %d %d\n", AT_ACK },
	{ "HILOWS\n", AT_ACK },
	{ "PUTRAIN %d\n", AT_ACK },
	{ "PUTET %d\n", AT_ACK },
	{ "DMP\n", AT_ACK },
	{ "DMPAFT\n", AT_ACK },
	{ "GETEE\n", AT_ACK },
	{ "EERD %hx %hu\n", AT_UNKOWN },
	{ "EEWR %hx %hu\n", AT_OK },
	{ "EEBRD %hx %hu\n", AT_ACK },
	{ "EEBWR %hx %hu\n", AT_ACK },
	{ "CALED\n", AT_ACK },
	{ "CALFIX\n", AT_ACK },
	{ "BAR=%d %d\n", AT_OK },
	{ "BARDATA\n", AT_OK },
	{ "CLRLOG\n", AT_OK },
	{ "CLRALM\n", AT_DONE },
	{ "CLRCAL\n", AT_DONE },
	{ "CLRGRA\n", AT_DONE },
	{ "CLRVAR %d\n", AT_ACK },
	{ "CLRHIGHS %d\n", AT_ACK },
	{ "CLRLOWS %d\n", AT_ACK },
	{ "CLRBITS\n", AT_ACK },
	{ "CLRDATA\n", AT_ACK },
	{ "BAUD %d\n", AT_OK },
	{ "SETTIME\n", AT_ACK },
	{ "GETTIME\n", AT_ACK },
	{ "GAIN %d\n", AT_OK },
	{ "SETPER %d\n", AT_ACK },
	{ "STOP\n", AT_UNKOWN },
	{ "START\n", AT_UNKOWN },
	{ "NEWSETUP\n", AT_ACK },
	{ "LAMPS %d\n", AT_OK },
};

static ssize_t
vantage_read_to(int fd, void *buf, size_t len, long timeout)
{
	ssize_t sz;

	sz = 0;

	while (sz < len) {
		ssize_t ret;

		if ((ret = ws_read_to(fd, buf + sz, len - sz, timeout)) == -1) {
			goto error;
		} else if (ret == 0) {
			errno = ETIME;
			goto error;
		}

		sz += ret;
	}

	return sz;

error:
	return -1;
}

static ssize_t
vantage_read(int fd, void *buf, size_t len)
{
	return vantage_read_to(fd, buf, len, IO_TIMEOUT);
}

static int
vantage_ack_ck_to(int fd, const uint8_t *ack, size_t acklen, long timeout)
{
	char buf[acklen];

	if (vantage_read_to(fd, buf, acklen, timeout) == -1) {
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

static int
vantage_ack_ck(int fd, const uint8_t *ref, size_t reflen)
{
	return vantage_ack_ck_to(fd, ref, reflen, IO_TIMEOUT);
}

static int
vantage_write_cmd(int fd, const char* cmd, size_t cmdlen)
{
	if (cmd[cmdlen - 1] != LF) {
		errno = EINVAL;
		goto error;
	}

	/* Write command */
	if (ws_write(fd, cmd, cmdlen) == -1) {
		goto error;
	}

	return 0;

error:
	return -1;
}

static int
vantage_read_ack(int fd, enum ack_type at)
{
	int ret;

	switch (at) {
	case AT_ACK:
		ret = vantage_ack_ck(fd, ACK_ACK, sizeof(ACK_ACK));
		break;
	case AT_OK:
		ret = vantage_ack_ck(fd, ACK_OK, sizeof(ACK_OK));
		break;
	case AT_TEST:
		ret = vantage_ack_ck(fd, ACK_TEST, sizeof(ACK_TEST));
		break;
	case AT_DONE:
		if (vantage_ack_ck(fd, ACK_OK, sizeof(ACK_OK)) == -1) {
			goto error;
		}
		ret = vantage_ack_ck_to(fd, ACK_DONE, sizeof(ACK_DONE), IO_DONE_TIMEOUT);
		break;
	default:
		errno = EINVAL;
		ret = -1;
	}

	return ret;

error:
	return -1;
}

int
vantage_proc(int fd, enum vantage_cmd cmd, /* args */ ...)
{
	va_list ap;
	char buf[CMD_MAX];
	int bufsz;

	if (cmd < 0 && cmd > LAMPS) {
		errno = EINVAL;
		goto error;
	}

	/* Send command */
	va_start(ap, cmd);
	bufsz = vsnprintf(buf, sizeof(buf), CMDS[cmd].fmt, ap);
	va_end(ap);

	if (vantage_write_cmd(fd, buf, bufsz) == -1) {
		goto error;
	}

	/* Read response */
	return vantage_read_ack(fd, CMDS[cmd].ack);

error:
	return -1;
}

int
vantage_ack(int fd, const char *cmd, size_t cmdlen, void *buf, size_t len)
{
	ssize_t sz;
	uint8_t ack;

	if (vantage_write_cmd(fd, cmd, cmdlen) == -1) {
		goto error;
	}

	/* ACK */
	if ((sz = vantage_read(fd, &ack, 1)) == -1) {
		goto error;
	}

	if (ack != ACK) {
		errno = EIO;
		goto error;
	}

	/* Data */
	return vantage_read(fd, buf, len);

error:
	return -1;
}

int
vantage_ok(int fd, const char *cmd, size_t cmdlen, void *buf, size_t len)
{

	if (vantage_write_cmd(fd, cmd, cmdlen) == -1) {
		goto error;
	}

	/* OK */
	if (vantage_ack_ck(fd, ACK_OK, sizeof(ACK_OK)) == -1) {
		goto error;
	}

	/* Data */
	if (len > 0) {
		if (vantage_read(fd, buf, len) == -1) {
			goto error;
		}
		if (vantage_ack_ck(fd, DELIM, sizeof(DELIM)) == -1) {
			goto error;
		}
	}

	return 0;

error:
	return -1;
}
