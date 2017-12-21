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
#include "libws/vantage/util.h"
#include "libws/vantage/vantage.h"

#define CMD_MAX		(8 + 2)			/* maximum length of command */

#define OK		"\n\rOK\n\r"		/* OK */

DSO_EXPORT int
vantage_test(int fd)
{
//	char out[6];
//	const char in[5] = "TEST";
//
//	memset(out, 0, sizeof(out));
//
//	if (ws_write(fd, in, sizeof(in)) == -1) {
//		goto error;
//	}
//	if (ws_read(fd, out, sizeof(out)) == -1) {
//		goto error;
//	}
//
//	/* check output */
//	if (memcmp(in, out, sizeof(in)) || out[5] != '\r') {
//		errno = EIO;
//		goto error;
//	}
//
//	return 0;
//
//error:
	return -1;
}

DSO_EXPORT int
vantage_ver(int fd, char *buf, size_t len)
{
	if (len < 12) {
		errno = EINVAL;
		return -1;
	}

	buf[11] = 0;

	return vantage_ok(fd, "VER\n", 4, buf, 11);
}

DSO_EXPORT int
vantage_wrd(int fd, enum vantage_type *wrd)
{
	char cmd[] = { 'W', 'R', 'D', 0x12, 0x4d, LF };
	uint8_t buf[1];

	if (vantage_ack(fd, cmd, sizeof(cmd), buf, sizeof(buf)) == -1) {
		goto error;
	}

	/* Decode result */
	*wrd = buf[0];

	return 0;

error:
	return -1;
}

DSO_EXPORT int
vantage_nver(int fd, char *buf, size_t len)
{
	if (len < 5) {
		errno = EINVAL;
		return -1;
	}

	buf[4] = 0;

	return vantage_ok(fd, "NVER\n", 5, buf, 4);
}

DSO_EXPORT int
vantage_rxcheck(int fd, struct vantage_rxck *r)
{
//	size_t sz;
//	char buf[60];
//
//	if ((sz = vantage_io(fd, "RXCHECK", buf, sizeof(buf))) == -1) {
//		goto error;
//	}
//
//
//	sscanf(buf, " %ld %ld %ld %ld %ld", &r->received, &r->missed, &r->resync, &r->in_row, &r->crc_ko);
//
//	return 0;
//
//error:
	return -1;
}
