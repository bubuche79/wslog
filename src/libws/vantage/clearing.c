/**
 * Clearing commands.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>

#include "libws/vantage/util.h"
#include "libws/vantage/vantage.h"

int
vantage_clrlog(int fd)
{
	return vantage_proc(fd, CLRLOG);
}

int
vantage_clralm(int fd)
{
	return vantage_proc(fd, CLRALM);
}

int
vantage_clrcal(int fd)
{
	return vantage_proc(fd, CLRCAL);
}

int
vantage_clrgra(int fd)
{
	return vantage_proc(fd, CLRGRA);
}

int
vantage_clrvar(int fd, enum vantage_var var)
{
	return vantage_proc(fd, CLRVAR, (int) var);
}

int
vantage_clrhighs(int fd, enum vantage_freq freq)
{
	return vantage_proc(fd, CLRHIGHS, (int) freq);
}

int
vantage_clrlows(int fd, enum vantage_freq freq)
{
	return vantage_proc(fd, CLRLOWS, (int) freq);
}

int
vantage_clrbits(int fd)
{
	return vantage_proc(fd, CLRBITS);
}

int
vantage_clrdata(int fd)
{
	return vantage_proc(fd, CLRDATA);
}
