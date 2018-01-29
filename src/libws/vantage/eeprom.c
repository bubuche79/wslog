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
	uint8_t buf[0xAF];

	if (vantage_eebrd(fd, 0x01, buf, sizeof(buf)) == -1) {
		goto error;
	}

	p->latitude = vantage_int16(buf, 0x0B - 0x01);
	p->longitude = vantage_int16(buf, 0x0D - 0x01);
	p->altitude = vantage_uint16(buf, 0x0F - 0x01);
	p->unit_bits = vantage_uint8(buf, 0x29 - 0x01);
	p->setup_bits = vantage_uint8(buf, 0x2B - 0x01);
	p->rain_start = vantage_uint8(buf, 0x2C - 0x01);
	p->ar_period = vantage_uint8(buf, 0x2D - 0x01);

	if (vantage_eebrd(fd, 0xFFC, buf, 1) == -1) {
		goto error;
	}

	p->log_avg = vantage_uint8(buf, 0);

	return 0;

error:
	return -1;
}
