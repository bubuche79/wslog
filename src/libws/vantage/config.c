/**
 * Configuration commands.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <time.h>
#include <errno.h>

#include "defs/dso.h"

#include "libws/vantage/util.h"
#include "libws/vantage/vantage.h"

DSO_EXPORT int
vantage_baud(int fd, speed_t speed)
{
	char cmd[12];
	int cmdlen;
	int bauds;

	if (speed == B1200) {
		bauds = 1200;
	} else if (speed == B2400) {
		bauds = 2400;
	} else if (speed == B4800) {
		bauds = 4800;
	} else if (speed == B9600) {
		bauds = 9600;
	} else if (speed == B19200) {
		bauds = 19200;
	} else {
		errno = EINVAL;
		goto error;
	}

	cmdlen = snprintf(cmd, sizeof(cmd), "BAUD %d\n", bauds);

	return vantage_ok(fd, cmd, cmdlen, NULL, 0);

error:
	return -1;
}

DSO_EXPORT int
vantage_settime(int fd, time_t time)
{
	struct tm tm;
	uint8_t buf[6];

	if (vantage_ack(fd, "SETTIME\n", 8, NULL, 0) == -1) {
		goto error;
	}

	/* Encode */
	if (localtime_r(&time, &tm) == NULL) {
		goto error;
	}

	buf[0] = tm.tm_sec;
	buf[1] = tm.tm_min;
	buf[2] = tm.tm_hour;
	buf[3] = tm.tm_mday;
	buf[4] = tm.tm_mon + 1;
	buf[5] = tm.tm_yday - 1900;

	/* Send with CRC */
	return vantage_write_crc(fd, buf, sizeof(buf));

error:
	return -1;
}

DSO_EXPORT int
vantage_gettime(int fd, time_t *time)
{
	struct tm tm;
	uint8_t buf[6];

	if (vantage_ack_crc(fd, "GETTIME\n", 8, buf, sizeof(buf)) == -1) {
		goto error;
	}

	/* Decode */
	tm.tm_sec = buf[0];
	tm.tm_min = buf[1];
	tm.tm_hour = buf[2];
	tm.tm_mday = buf[3];
	tm.tm_mon = buf[4] - 1;
	tm.tm_yday = 1900 + buf[5];

	*time = mktime(&tm);

	return 0;

error:
	return -1;
}

DSO_EXPORT int
vantage_gain(int fd, int on)
{
	return -1;
}

DSO_EXPORT int
vantage_setper(int fd, unsigned int min)
{
	char cmd[12];
	int cmdlen;

	switch (min) {
	case 1:
	case 5:
	case 10:
	case 15:
	case 30:
	case 60:
	case 120:
		break;
	default:
		errno = EINVAL;
		goto error;
	}

	cmdlen = snprintf(cmd, sizeof(cmd), "SETPER %d\n", min);

	return vantage_ok(fd, cmd, cmdlen, NULL, 0);

error:
	return -1;
}

DSO_EXPORT int
vantage_stop(int fd)
{
	return -1;
}

DSO_EXPORT int
vantage_start(int fd)
{
	return -1;
}

DSO_EXPORT int
vantage_newsetup(int fd)
{
	return vantage_ack(fd, "NEWSETUP\n", 9, NULL, 0);
}

DSO_EXPORT int
vantage_lamps(int fd, int on)
{
	char cmd[9];
	int cmdlen;

	cmdlen = snprintf(cmd, sizeof(cmd), "LAMPS %d\n", on ? 1 : 0);

	return vantage_ok(fd, cmd, cmdlen, NULL, 0);
}
