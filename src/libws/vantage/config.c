/**
 * Configuration commands.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "defs/dso.h"

#include "libws/vantage/vantage.h"

DSO_EXPORT int
vantage_baud(int fd, speed_t speed)
{
	char cmd[12];

	snprintf(cmd, sizeof(cmd), "BAUD %d\n", speed);

	return -1;
}

DSO_EXPORT int
vantage_gettime(int fd, time_t *time)
{
	return -1;
}
