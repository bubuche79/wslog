/**
 * Testing commands.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>

#include "defs/dso.h"

#include "libws/vantage/util.h"
#include "libws/vantage/vantage.h"

DSO_EXPORT ssize_t
vantage_getee(int fd, void *buf, size_t len)
{
	if (len < EEPROM_SIZE) {
		errno = EINVAL;
		goto error;
	}

	/* GETEE command */
	if (vantage_proc(fd, GETEE) == -1) {
		goto error;
	}

	return vantage_pread(fd, IO_CRC, buf, EEPROM_SIZE);

error:
	return -1;
}

DSO_EXPORT ssize_t
vantage_eerd(int fd, uint16_t addr, void *buf, size_t len)
{
	errno = ENOTSUP;
	return -1;
}

DSO_EXPORT ssize_t
vantage_eewr(int fd, uint16_t addr, void *buf, size_t len)
{
	errno = ENOTSUP;
	return -1;
}

DSO_EXPORT ssize_t
vantage_eebrd(int fd, uint16_t addr, void *buf, size_t len)
{
	if (EEPROM_SIZE < addr + len) {
		errno = EINVAL;
		goto error;
	}

	/* EEBRD command */
	if (vantage_proc(fd, EEBRD, addr, len) == -1) {
		goto error;
	}

	return vantage_pread(fd, IO_CRC, buf, len);

error:
	return -1;
}

DSO_EXPORT ssize_t
vantage_eebwr(int fd, uint16_t addr, void *buf, size_t len)
{
	errno = ENOTSUP;
	return -1;
}
