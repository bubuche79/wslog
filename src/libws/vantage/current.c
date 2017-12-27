/**
 * Current commands.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>

#include "defs/dso.h"

#include "libws/vantage/util.h"
#include "libws/vantage/vantage.h"

#define CMD_LOOP 	99
#define CMD_HILOWS	436

DSO_EXPORT ssize_t
vantage_loop(int fd, struct vantage_loop *buf, size_t nel)
{
	return -1;
}

DSO_EXPORT ssize_t
vantage_lps(int fd, int type, struct vantage_loop *buf, size_t nel)
{
	return -1;
}

DSO_EXPORT ssize_t
vantage_hilows(int fd, struct vantage_hilow *buf, size_t nel)
{
	return -1;
}

DSO_EXPORT ssize_t
vantage_putrain(int fd, long rain)
{
	if (rain < 0) {
		errno = EINVAL;
		goto error;
	}

	return vantage_proc(fd, PUTRAIN, rain);

error:
	return -1;
}

DSO_EXPORT ssize_t
vantage_putet(int fd, long et)
{
	if (et < 0) {
		errno = EINVAL;
		goto error;
	}

	return vantage_proc(fd, PUTET, et);

error:
	return -1;
}
