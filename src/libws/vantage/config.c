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

	return vantage_proc(fd, BAUD, bauds);

error:
	return -1;
}

DSO_EXPORT int
vantage_settime(int fd, time_t time)
{
	struct tm tm;
	uint8_t buf[6];

	/* SETTIME command */
	if (vantage_proc(fd, SETTIME) == -1) {
		goto error;
	}

	/* Write timestamp */
	if (localtime_r(&time, &tm) == NULL) {
		goto error;
	}

	buf[0] = tm.tm_sec;
	buf[1] = tm.tm_min;
	buf[2] = tm.tm_hour;
	buf[3] = tm.tm_mday;
	buf[4] = tm.tm_mon + 1;
	buf[5] = tm.tm_yday - 1900;

	return vantage_pwrite(fd, IO_CRC|IO_ACK, buf, sizeof(buf));

error:
	return -1;
}

DSO_EXPORT int
vantage_gettime(int fd, time_t *time)
{
	struct tm tm;
	uint8_t buf[6];

	/* GETTIME command */
	if (vantage_proc(fd, GETTIME) == -1) {
		goto error;
	}

	/* Read timestamp */
	if (vantage_pread(fd, IO_CRC, buf, sizeof(buf)) == -1) {
		goto error;
	}

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
	return vantage_proc(fd, GAIN, (on) ? 1 : 0);
}

DSO_EXPORT int
vantage_setper(int fd, int min)
{
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

	return vantage_proc(fd, SETPER, min);

error:
	return -1;
}

DSO_EXPORT int
vantage_stop(int fd)
{
	errno = ENOTSUP;
	return -1;
}

DSO_EXPORT int
vantage_start(int fd)
{
	errno = ENOTSUP;
	return -1;
}

DSO_EXPORT int
vantage_newsetup(int fd)
{
	return vantage_proc(fd, NEWSETUP);
}

DSO_EXPORT int
vantage_lamps(int fd, int on)
{
	return vantage_proc(fd, LAMPS, (on) ? 1 : 0);
}
