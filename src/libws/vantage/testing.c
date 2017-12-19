/**
 * Testing commands.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include "defs/dso.h"

#include "libws/serial.h"
#include "libws/vantage/vantage.h"

#define CMD_MAX		(8 + 2)			/* maximum length of command */

#define ACK		0x06			/* acknowledge */
#define NACK		0x21			/* not acknowledge */
#define CANCEL		0x18			/* CRC check error */
#define OK		"\n\rOK\n\r"		/* OK */

static int
vantage_resp_ok(const char *resp, size_t rlen, char *buf, size_t len)
{
	int res;

	if (rlen < 8) {
		res = -1;
	} else if (!memcmp(resp, "\n\rOK\n\r", 6)) {
		memcpy(buf, resp + 6, rlen - 6);
	}

	return res;
}

static ssize_t
vantage_io(int fd, const char *cmd, char *buf, size_t len)
{
	ssize_t cmd_len;
	char cmd_buf[CMD_MAX];
	ssize_t sz;

	/* command */
	cmd_len = snprintf(cmd_buf, CMD_MAX, "%s\n", cmd);
	if (cmd_len == CMD_MAX) {
		errno = EINVAL;
		goto error;
	}

	/* send command */
	if (ws_write(fd, cmd_buf, cmd_len) == -1) {
		goto error;
	}

	if ((sz = ws_read(fd, buf, len)) == -1) {
		goto error;
	}

	return sz;

error:
	return -1;
}

/**
 * Parse response of the form "\n\rOK\n\rDATA\n\r".
 */
static ssize_t
vantage_prefix_ok(int fd, const char *cmd, char *buf, size_t len)
{
	ssize_t sz;
	ssize_t res;
	char out[len + 8];

	if ((sz = vantage_io(fd, cmd, out, sizeof(out))) == -1) {
		goto error;
	}

	/* check response */
	if ((res = vantage_resp_ok(out, sz, buf, len)) == -1) {
		goto error;
	}

	return res;

error:
	return -1;
}

DSO_EXPORT int
vantage_test(int fd)
{
	char out[6];
	const char in[5] = "TEST";

	memset(out, 0, sizeof(out));

	if (ws_write(fd, in, sizeof(in)) == -1) {
		goto error;
	}
	if (ws_read(fd, out, sizeof(out)) == -1) {
		goto error;
	}

	/* check output */
	if (memcmp(in, out, sizeof(in)) || out[5] != '\r') {
		errno = EIO;
		goto error;
	}

	return 0;

error:
	return -1;
}

DSO_EXPORT int
vantage_ver(int fd, char *buf, size_t len)
{
	return vantage_prefix_ok(fd, "VER", buf, len);
}

DSO_EXPORT int
vantage_nver(int fd, char *buf, size_t len)
{
	return vantage_prefix_ok(fd, "NVER", buf, len);
}

DSO_EXPORT int
vantage_rxcheck(int fd, struct vantage_rxck *r)
{
	size_t sz;
	char buf[60];

	if ((sz = vantage_io(fd, "RXCHECK", buf, sizeof(buf))) == -1) {
		goto error;
	}


	sscanf(buf, " %ld %ld %ld %ld %ld", &r->received, &r->missed, &r->resync, &r->in_row, &r->crc_ko);

	return 0;

error:
	return -1;
}
