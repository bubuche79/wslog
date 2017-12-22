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

DSO_EXPORT int
vantage_test(int fd)
{
	return vantage_proc(fd, TEST);
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

	buf[11] = 0;

	return vantage_ok(fd, "VER\n", 4, buf, 11);
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

	buf[4] = 0;

	return vantage_ok(fd, "NVER\n", 5, buf, 4);
}
