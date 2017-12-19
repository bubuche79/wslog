/**
 * Download commands.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <stdint.h>

#include "defs/dso.h"

#include "libws/vantage/vantage.h"

#define DMP_SIZE	52

DSO_EXPORT ssize_t
vantage_dmp(int fd, struct vantage_dmp *buf, size_t nel)
{
//	uint8_t buf[DMP_SIZE] = { 0 };
//	struct tm tm;
//
//	/* timestamp */
//	tm.tm_year = 2000 + (buf[0] >> 1);
//	tm.tm_mon = (buf[0] & 0x1) | buf[1] >> 5;
//	tm.tm_mday = buf[1] & 0x1F;
//	h->tstamp = mktime(&tm);

	return -1;
}

DSO_EXPORT ssize_t
vantage_dmpaft(int fd, struct vantage_dmp *buf, size_t nel, time_t after)
{
	return -1;
}
