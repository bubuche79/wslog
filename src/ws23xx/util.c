#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <time.h>
#include <string.h>

#include "libws/ws23xx/decoder.h"

#include "util.h"

char *
ws23xx_temp_str(const uint8_t *buf, char *s, size_t len, size_t offset)
{
	float v;

	ws23xx_temp(buf, &v, offset);
	snprintf(s, len, "%.2f", v);

	return s;
}

char *
ws23xx_pressure_str(const uint8_t *buf, char *s, size_t len, size_t offset)
{
	float v;

	ws23xx_pressure(buf, &v, offset);
	snprintf(s, len, "%.1f", v);

	return s;
}

char *
ws23xx_humidity_str(const uint8_t *buf, char *s, size_t len, size_t offset)
{
	uint8_t v;

	ws23xx_humidity(buf, &v, offset);
	snprintf(s, len, "%hhu", v);

	return s;
}

char *
ws23xx_rain_str(const uint8_t *buf, char *s, size_t len, size_t offset)
{
	float v;

	ws23xx_rain(buf, &v, offset);
	snprintf(s, len, "%.2f", v);

	return s;
}

char *
ws23xx_speed_str(const uint8_t *buf, char *s, size_t len, size_t offset)
{
	float v;

	ws23xx_speed(buf, &v, offset);
	snprintf(s, len, "%.1f", v);

	return s;
}

char *
ws23xx_wind_dir_str(const uint8_t *buf, char *s, size_t len, size_t offset)
{
	uint16_t v;

	ws23xx_wind_dir(buf, &v, offset);
	snprintf(s, len, "%hu", v);

	return s;
}

char *
ws23xx_interval_sec_str(const uint8_t *buf, char *s, size_t len, size_t offset)
{
	float v;

	ws23xx_interval_sec(buf, &v, offset);
	snprintf(s, len, "%.1f", v);

	return s;
}

char *
ws23xx_interval_min_str(const uint8_t *buf, char *s, size_t len, size_t offset)
{
	uint16_t v;

	ws23xx_interval_min(buf, &v, offset);
	snprintf(s, len, "%hu", v);

	return s;
}

char *
ws23xx_bin_2nyb_str(const uint8_t *buf, char *s, size_t len, size_t offset)
{
	uint8_t v;

	ws23xx_bin_2nyb(buf, &v, offset);
	snprintf(s, len, "%hhu", v);

	return s;
}

char *
ws23xx_datetime_str(const uint8_t *buf, char *s, size_t len, size_t offset)
{
	time_t ts;
	struct tm tm;

	ws23xx_datetime(buf, &ts, offset);
	localtime_r(&ts, &tm);
	strftime(s, len, "%Y-%m-%d %H:%M", &tm);

	return s;
}

char *
ws23xx_timestamp_str(const uint8_t *buf, char *s, size_t len, size_t offset)
{
	time_t ts;
	struct tm tm;

	ws23xx_timestamp(buf, &ts, offset);
	localtime_r(&ts, &tm);
	strftime(s, len, "%Y-%m-%d %H:%M", &tm);

	return s;
}
