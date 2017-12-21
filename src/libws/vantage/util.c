#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <errno.h>

#include "libws/serial.h"
#include "libws/vantage/util.h"
#include "libws/vantage/vantage.h"

#define CMD_MAX		(8 + 2)		/* maximum length of command */
#define IO_TIMEOUT	250		/* read timeout, in milliseconds */

static const uint8_t DELIM[] = { LF, CR };
static const uint8_t OK[] = { LF, CR, 'O', 'K', LF, CR };

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
	return vantage_read_to(fd, buf, len, 250);
}

static int
vantage_read_check(int fd, const uint8_t *ref, size_t reflen)
{
	char buf[reflen];

	if (vantage_read(fd, buf, reflen) == -1) {
		goto error;
	}

	if (memcmp(ref, buf, reflen)) {
		errno = EIO;
		goto error;
	}

	return 0;

error:
	return -1;
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
	if (vantage_read_check(fd, OK, sizeof(OK)) == -1) {
		goto error;
	}

	/* Data */
	if (len > 0) {
		if (vantage_read(fd, buf, len) == -1) {
			goto error;
		}
		if (vantage_read_check(fd, DELIM, sizeof(DELIM)) == -1) {
			goto error;
		}
	}

	return 0;

error:
	return -1;
}
