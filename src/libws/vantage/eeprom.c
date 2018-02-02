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

static uint8_t
vantage_bits(uint8_t byte, size_t off, size_t len)
{
	return (byte >> off) & ((0x1 << (len + 1)) - 1);
}

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
	uint8_t unit_bits, setup_bits;

	if (vantage_eebrd(fd, 0x01, buf, sizeof(buf)) == -1) {
		goto error;
	}

	p->latitude = vantage_int16(buf, 0x0B - 0x01);
	p->longitude = vantage_int16(buf, 0x0D - 0x01);
	p->altitude = vantage_uint16(buf, 0x0F - 0x01);
	unit_bits = vantage_uint8(buf, 0x29 - 0x01);
	setup_bits = vantage_uint8(buf, 0x2B - 0x01);
	p->rain_start = vantage_uint8(buf, 0x2C - 0x01);
	p->ar_period = vantage_uint8(buf, 0x2D - 0x01);
	p->in_temp_cal = vantage_int8(buf, 0x32 - 0x01);
	p->temp_cal = vantage_int8(buf, 0x34 - 0x01);
	p->in_humidity_cal = vantage_int8(buf, 0x44 - 0x01);
	p->humidity_cal = vantage_int8(buf, 0x45 - 0x01);
	p->dir_cal = vantage_int16(buf, 0x4D - 0x01);

	/* Decode bit fields */
	p->ub.barometer = vantage_bits(unit_bits, 0, 2);
	p->ub.temp = vantage_bits(unit_bits, 2, 2);
	p->ub.altitude = vantage_bits(unit_bits, 4, 1);
	p->ub.rain = vantage_bits(unit_bits, 5, 1);
	p->ub.wind = vantage_bits(unit_bits, 6, 2);

	p->sb.time_mode = vantage_bits(setup_bits, 0, 1);
	p->sb.day_period = vantage_bits(setup_bits, 1, 1);
	p->sb.month_fmt = vantage_bits(setup_bits, 2, 1);
	p->sb.wind_cup = vantage_bits(setup_bits, 3, 1);
	p->sb.rain_cup = vantage_bits(setup_bits, 4, 2);
	p->sb.latitude = vantage_bits(setup_bits, 6, 1);
	p->sb.longitude = vantage_bits(setup_bits, 7, 1);

	if (vantage_eebrd(fd, 0xFFC, buf, 1) == -1) {
		goto error;
	}

	p->log_avg = vantage_uint8(buf, 0);

	return 0;

error:
	return -1;
}
