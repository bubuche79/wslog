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

#define LOOP_SIZE 	97		/* 99 bytes with CRC */
#define LOOP_DELAY 	2000

#define LPS_DELAY 	2500		/* LPS delay between packets */
#define LPS_MASK	0x03		/* LOOP and LOOP2 packets */

static void
vantage_loop_decode(struct vantage_loop *loop, const uint8_t *buf)
{
	loop->bar_trend = vantage_int8(buf, 3);
	loop->barometer = vantage_uint16(buf, 7);
	loop->in_temp = vantage_int16(buf, 9);
	loop->in_humidity = vantage_uint8(buf, 11);
	loop->temp = vantage_int16(buf, 12);
	loop->wind_speed = vantage_uint8(buf, 11);
	loop->wind_dir = vantage_uint16(buf, 16);
	loop->wind_avg_10m = vantage_uint16(buf, 18);
	loop->wind_avg_2m = vantage_uint16(buf, 20);
	loop->wind_gust_10m = vantage_uint16(buf, 22);
	loop->wind_gust_dir_10m = vantage_uint16(buf, 24);
	loop->dew_point = vantage_int16(buf, 30);
	loop->humidity = vantage_uint8(buf, 33);
	loop->heat_index = vantage_int16(buf, 35);
	loop->wind_chill = vantage_int16(buf, 37);
	loop->thsw_chill = vantage_int16(buf, 39);
	loop->rain_rate = vantage_int16(buf, 41);
	loop->uv = vantage_int8(buf, 43);
	loop->solar_rad = vantage_int16(buf, 44);
	loop->storm_rain = vantage_int16(buf, 46);
	loop->storm_start = vantage_mktime(buf, 48, 0);
	loop->daily_rain = vantage_int16(buf, 50);
	loop->last_15m_rain = vantage_int16(buf, 52);
	loop->last_1h_rain = vantage_int16(buf, 54);
	loop->daily_et = vantage_int16(buf, 56);
	loop->last_24h_rain = vantage_int16(buf, 58);
	loop->barometer_algo = vantage_int8(buf, 60);
	loop->barometer_off = vantage_int16(buf, 61);
	loop->barometer_cal = vantage_int16(buf, 63);
	loop->barometer_raw = vantage_int16(buf, 65);
	loop->barometer_abs = vantage_int16(buf, 67);
	loop->altimeter_opts = vantage_int16(buf, 69);
}

DSO_EXPORT ssize_t
vantage_loop(int fd, struct vantage_loop *p, size_t nel)
{
	errno = ENOTSUP;
	return -1;
}

DSO_EXPORT ssize_t
vantage_lps(int fd, int type, struct vantage_loop *p, size_t nel)
{
	ssize_t sz;
	uint8_t buf[LOOP_SIZE];
	long timeout;

	timeout = 0;

	if (type & LPS_LOOP) {
		errno = EINVAL;
		goto error;
	}

	type = type & LPS_MASK;

	/* LPS command */
	if (vantage_proc(fd, LPS, type, nel) == -1) {
		goto error;
	}

	/* Read LOOP records */
	timeout = IO_TIMEOUT;

	for (sz = 0; sz < nel; sz++) {
		if (vantage_pread_to(fd, IO_CRC, buf, sizeof(buf), timeout) == -1) {
			goto error;
		}

		vantage_loop_decode(&p[sz], buf);

		if (sz == 0) {
			timeout += LPS_DELAY;
		}
	}

	return sz;

error:
	if (timeout > 0) {
		uint8_t cr = CR;

		/* Cancel */
		(void) vantage_write(fd, &cr, 1);
	}

	return -1;
}

DSO_EXPORT ssize_t
vantage_hilows(int fd, struct vantage_hilow *p, size_t nel)
{
	errno = ENOTSUP;
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
