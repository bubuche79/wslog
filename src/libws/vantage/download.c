/**
 * Download commands.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <time.h>
#include <stdint.h>

#include "defs/dso.h"

#include "libws/vantage/vantage.h"

#define RECORD_SIZE	52
#define PAGE_SIZE	264

static uint8_t
vantage_uint8(const uint8_t *buf)
{
	return buf[0];
}

static void
vantage_uint8_arr(const uint8_t *buf, size_t nel, uint8_t *arr)
{
	size_t i;

	for (i = 0; i < nel; i++) {
		arr[i] = vantage_uint8(buf + i);
	}
}

static uint16_t
vantage_uint16(const uint8_t *buf)
{
	return (buf[1] << 8) | buf[0];
}

static time_t
vantage_time(const uint8_t *buf)
{
	struct tm tm;
	uint16_t date, time;

	date = vantage_uint16(buf + 0);
	time = vantage_uint16(buf + 2);

	tm.tm_year = date >> 9;
	tm.tm_mon = (date >> 5) & 0x0F;
	tm.tm_mday = date & 0x1F;

	tm.tm_hour = time / 100;
	tm.tm_min = time - (100 * tm.tm_hour);

	return mktime(&tm);
}

static void
vantage_dmp_decode(struct vantage_dmp *d, const uint8_t *buf)
{
#if 0
	uint8_t rec_type;
#endif

	d->tstamp = vantage_time(buf + 0);
	d->temp = vantage_uint16(buf + 4);
	d->hi_temp = vantage_uint16(buf + 6);
	d->lo_temp = vantage_uint16(buf + 8);
	d->rain = vantage_uint16(buf + 10);
	d->hi_rain_rate = vantage_uint16(buf + 12);
	d->barometer = vantage_uint16(buf + 14);
	d->solar_rad = vantage_uint16(buf + 16);
	d->wind_samples = vantage_uint16(buf + 18);
	d->in_temp = vantage_uint16(buf + 20);
	d->in_humidity = vantage_uint8(buf + 22);
	d->humidity = vantage_uint8(buf + 23);
	d->avg_wind_speed = vantage_uint8(buf + 24);
	d->hi_wind_speed = vantage_uint8(buf + 25);
	d->hi_wind_dir = vantage_uint8(buf + 26);
	d->main_wind_dir = vantage_uint8(buf + 27);
	d->avg_uv = vantage_uint8(buf + 28);
	d->et = vantage_uint8(buf + 29);
	d->hi_solar_rad = vantage_uint16(buf + 30);
	d->hi_uv = vantage_uint8(buf + 32);
	d->forecast = vantage_uint8(buf + 33);

	vantage_uint8_arr(buf + 34, 2, d->leaf_temp);
	vantage_uint8_arr(buf + 36, 2, d->leaf_wet);
	vantage_uint8_arr(buf + 38, 4, d->soil_temp);

#if 0
	rec_type = vantage_uint8(buf + 42);
#endif

	vantage_uint8_arr(buf + 43, 2, d->extra_humidity);
	vantage_uint8_arr(buf + 45, 3, d->extra_temp);
	vantage_uint8_arr(buf + 48, 4, d->soil_moisture);
}

DSO_EXPORT ssize_t
vantage_dmp(int fd, struct vantage_dmp *buf, size_t nel)
{

	vantage_dmp_decode(buf, NULL);

	return -1;
}

DSO_EXPORT ssize_t
vantage_dmpaft(int fd, struct vantage_dmp *buf, size_t nel, time_t after)
{
	return -1;
}
