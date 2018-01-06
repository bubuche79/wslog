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

DSO_EXPORT int
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

DSO_EXPORT int
vantage_eerd(int fd, uint16_t addr, void *buf, size_t len)
{
	errno = ENOTSUP;
	return -1;
}

DSO_EXPORT int
vantage_eewr(int fd, uint16_t addr, uint8_t byte)
{
	errno = ENOTSUP;
	return -1;
}

DSO_EXPORT int
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

DSO_EXPORT int
vantage_eebwr(int fd, uint16_t addr, void *buf, size_t len)
{
	errno = ENOTSUP;
	return -1;
}

DSO_EXPORT int
vantage_ee_cfg(int fd, struct vantage_cfg *p)
{
	uint16_t addr;
	uint8_t buf[3];

	addr = 0x2B;

	if (vantage_eebrd(fd, addr, buf, sizeof(buf)) == -1) {
		goto error;
	}

	p->setup = buf[0x2B - addr];
	p->rain_start = buf[0x2C - addr];
	p->ar_period = buf[0x2D - addr];

	return 0;

error:
	return -1;
}
