/**
 * Clearing commands.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "defs/dso.h"

#include "libws/serial.h"
#include "libws/vantage/util.h"
#include "libws/vantage/vantage.h"

static int
vantage_clr(int fd, const char *cmd, int arg)
{
	char buf[16];
	int buflen;

	buflen = snprintf(buf, "%s %d\n", cmd, arg);
	return vantage_ack(fd, buf, buflen, NULL, 0);
}

DSO_EXPORT int
vantage_clrlog(int fd)
{
	return vantage_ack(fd, "CLRLOG\n", 7, NULL, 0);
}

DSO_EXPORT int
vantage_clralm(int fd)
{
	return -1;
}

DSO_EXPORT int
vantage_clrcal(int fd)
{
	return -1;
}

DSO_EXPORT int
vantage_clrgra(int fd)
{
	return -1;

}

DSO_EXPORT int
vantage_clrvar(int fd, enum vantage_var var)
{
	return vantage_clr(fd, "CLRVAR", var);
}

DSO_EXPORT int
vantage_clrhighs(int fd, enum vantage_freq freq)
{
	return vantage_clr(fd, "CLRHIGHS", freq);
}

DSO_EXPORT int
vantage_clrlows(int fd, enum vantage_freq freq)
{
	return vantage_clr(fd, "CLRLOWS", freq);
}

DSO_EXPORT int
vantage_clrbits(int fd)
{
	return vantage_ack(fd, "CLRBITS\n", 8, NULL, 0);
}

DSO_EXPORT int
vantage_clrdata(int fd)
{
	return vantage_ack(fd, "CLRDATA\n", 8, NULL, 0);
}
