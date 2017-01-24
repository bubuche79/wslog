#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <time.h>
#include <string.h>

#include "defs/dso.h"
#include "libws/nybble.h"
#include "libws/ws23xx/decoder.h"

static uint8_t
bit2num(const uint8_t *buf, size_t offset, uint8_t bit) {
	return nybget(buf, offset) & (1 << bit);
}

DSO_EXPORT uint8_t
ws23xx_bit(const uint8_t *buf, size_t offset, uint8_t bit)
{
	return bit2num(buf, offset, bit);
}

DSO_EXPORT float
ws23xx_temp(const uint8_t *buf, float *v, size_t offset)
{
	float res = ((long int) nybtoul(buf, 4, offset, 10) - 3000) / 100.0;

	if (v) {
		*v = res;
	}

	return res;
}

DSO_EXPORT float
ws23xx_pressure(const uint8_t *buf, float *v, size_t offset)
{
	float res = nybtoul(buf, 5, offset, 10) / 10.0;

	if (v) {
		*v = res;
	}

	return res;
}

DSO_EXPORT uint8_t
ws23xx_humidity(const uint8_t *buf, uint8_t *v, size_t offset)
{
	uint8_t res = (uint8_t) nybtoul(buf, 2, offset, 10);

	if (v) {
		*v = res;
	}

	return res;
}

DSO_EXPORT float
ws23xx_rain(const uint8_t *buf, float *v, size_t offset)
{
	float res = nybtoul(buf, 6, offset, 10) / 100.0;

	if (v) {
		*v = res;
	}

	return res;
}

DSO_EXPORT float
ws23xx_speed(const uint8_t *buf, float *v, size_t offset)
{
	float res = nybtoul(buf, 3, offset, 16) / 10.0;

	if (v) {
		*v = res;
	}

	return res;
}

#if 0
float
ws23xx_speed(const uint8_t *buf, float *v, size_t offset)
{
	float res = nybtoul(buf, 2, offset, 16) / 10.0 + nybtoul(buf + 1, 2, offset, 16) * 22.5;

	if (v) {
		*v = res;
	}

	return res;
}
#endif

DSO_EXPORT uint16_t
ws23xx_wind_dir(const uint8_t *buf, uint16_t *v, size_t offset)
{
	uint16_t res = nybget(buf, offset) * 22.5;

	if (v) {
		*v = res;
	}

	return res;
}

DSO_EXPORT uint8_t
ws23xx_wind_valid(const uint8_t *buf, uint8_t *v, size_t offset)
{
	uint8_t res = nybget(buf, offset);

	if (v) {
		*v = res;
	}

	return res;
}

DSO_EXPORT float
ws23xx_interval_sec(const uint8_t *buf, float *v, size_t offset)
{
	float res = (float) nybtoul(buf, 2, offset, 16) * 0.5;

	if (v) {
		*v = res;
	}

	return res;
}

DSO_EXPORT uint16_t
ws23xx_interval_min(const uint8_t *buf, uint16_t *v, size_t offset)
{
	uint16_t res = (uint16_t) nybtoul(buf, 3, offset, 16);

	if (v) {
		*v = res;
	}

	return res;
}

DSO_EXPORT uint8_t
ws23xx_bin_2nyb(const uint8_t *buf, uint8_t *v, size_t offset)
{
	uint8_t res = (uint8_t) nybtoul(buf, 2, offset, 16);

	if (v) {
		*v = res;
	}

	return res;
}

DSO_EXPORT time_t
ws23xx_datetime(const uint8_t *buf, time_t *v, size_t offset)
{
	time_t res;
	struct tm tm;

	memset(&tm, 0, sizeof(tm));

	tm.tm_min = nybtoul(buf, 2, offset, 10);
	tm.tm_hour = nybtoul(buf, 2, offset + 2, 10);
	tm.tm_wday = nybtoul(buf, 1, offset + 4, 10) - 1;
	tm.tm_mday = nybtoul(buf, 2, offset + 5, 10);
	tm.tm_mon = nybtoul(buf, 2, offset + 7, 10) - 1;
	tm.tm_year = nybtoul(buf, 2, offset + 9, 10) + 100;
	tm.tm_isdst = -1;

	res = mktime(&tm);

	if (v) {
		*v = res;
	}

	return res;
}

DSO_EXPORT time_t
ws23xx_timestamp(const uint8_t *buf, time_t *v, size_t offset)
{
	time_t res;
	struct tm tm;

	memset(&tm, 0, sizeof(tm));

	tm.tm_min = nybtoul(buf, 2, offset, 10);
	tm.tm_hour = nybtoul(buf, 2, offset + 2, 10);
	tm.tm_mday = nybtoul(buf, 2, offset + 4, 10);
	tm.tm_mon = nybtoul(buf, 2, offset + 6, 10) - 1;
	tm.tm_year = nybtoul(buf, 2, offset + 8, 10) + 100;
	tm.tm_isdst = -1;

	res = mktime(&tm);

	if (v) {
		*v = res;
	}

	return res;
}

DSO_EXPORT uint8_t
ws23xx_connection(const uint8_t *buf, uint8_t *v, size_t offset)
{
	uint8_t res = nybget(buf, offset);

	if (v) {
		*v = res;
	}

	return res;
}

DSO_EXPORT uint8_t
ws23xx_alarm_set(const uint8_t *buf, uint8_t *v, size_t offset, uint8_t bit)
{
	uint8_t res = bit2num(buf, offset, bit);

	if (v) {
		*v = res;
	}

	return res;
}

DSO_EXPORT uint8_t
ws23xx_alarm_active(const uint8_t *buf, uint8_t *v, size_t offset, uint8_t bit)
{
	uint8_t res = bit2num(buf, offset, bit);

	if (v) {
		*v = res;
	}

	return res;
}
