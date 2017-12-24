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

static int
vantage_verx(int fd, enum vantage_cmd cmd, char *buf, size_t len)
{
	/* Vantage command */
	if (vantage_proc(fd, cmd) == -1) {
		goto error;
	}

	/* Read version */
	if (vantage_read(fd, buf, len) == -1) {
		goto error;
	}

	buf[len] = 0;

	return 0;

error:
	return -1;
}

DSO_EXPORT int
vantage_test(int fd)
{
	return vantage_proc(fd, TEST);
}

DSO_EXPORT int
vantage_wrd(int fd, enum vantage_type *wrd)
{
	uint8_t byte;

	/* WRD command */
	if (vantage_proc(fd, WRD, 0x12, 0x4d) == -1) {
		goto error;
	}

	/* Read response */
	if (vantage_read(fd, &byte, sizeof(byte)) == -1) {
		goto error;
	}

	*wrd = byte;

	return 0;

error:
	return -1;
}

DSO_EXPORT int
vantage_rxcheck(int fd, struct vantage_rxck *ck)
{
	return -1;
}

DSO_EXPORT int
vantage_rxtest(int fd)
{
	return -1;
}

DSO_EXPORT int
vantage_ver(int fd, char *buf, size_t len)
{
	if (len < 12) {
		errno = EINVAL;
		return -1;
	}

	return vantage_verx(fd, VER, buf, 11);
}

DSO_EXPORT int
vantage_receivers(int fd, uint8_t *receivers)
{
	return -1;
}

DSO_EXPORT int
vantage_nver(int fd, char *buf, size_t len)
{
	if (len < 5) {
		errno = EINVAL;
		return -1;
	}

	return vantage_verx(fd, NVER, buf, 4);
}
