/**
 * Clearing commands.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>

#include "defs/dso.h"

#include "libws/vantage/util.h"
#include "libws/vantage/vantage.h"

DSO_EXPORT int
vantage_clrlog(int fd)
{
	return vantage_proc(fd, CLRLOG);
}

DSO_EXPORT int
vantage_clralm(int fd)
{
	errno = ENOTSUP;
	return -1;
}

DSO_EXPORT int
vantage_clrcal(int fd)
{
	errno = ENOTSUP;
	return -1;
}

DSO_EXPORT int
vantage_clrgra(int fd)
{
	errno = ENOTSUP;
	return -1;
}

DSO_EXPORT int
vantage_clrvar(int fd, enum vantage_var var)
{
	return vantage_proc(fd, CLRVAR, (int) var);
}

DSO_EXPORT int
vantage_clrhighs(int fd, enum vantage_freq freq)
{
	return vantage_proc(fd, CLRHIGHS, (int) freq);
}

DSO_EXPORT int
vantage_clrlows(int fd, enum vantage_freq freq)
{
	return vantage_proc(fd, CLRLOWS, (int) freq);
}

DSO_EXPORT int
vantage_clrbits(int fd)
{
	return vantage_proc(fd, CLRBITS);
}

DSO_EXPORT int
vantage_clrdata(int fd)
{
	return vantage_proc(fd, CLRDATA);
}
