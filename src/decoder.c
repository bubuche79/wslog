#include <stdio.h>
#include <time.h>
#include <string.h>

#include "core/nybble.h"

#include "decoder.h"

static uint8_t
bit2num(const uint8_t *buf, size_t offset, uint8_t bit) {
	return nybat(buf, offset) & (1 << bit);
}

uint8_t
ws_bit(const uint8_t *buf, size_t offset, uint8_t bit)
{
	return bit2num(buf, offset, bit);
}

double *
ws_temp(const uint8_t *buf, double *v, size_t offset)
{
	*v = ((int64_t) nybdtol(buf, 4, offset) - 3000) / 100.0;

	return v;
}

char *
ws_temp_str(const uint8_t *buf, char *s, size_t len, size_t offset)
{
	double v;

	ws_temp(buf, &v, offset);
	snprintf(s, len, "%.2f", v);

	return s;
}

double *
ws_pressure(const uint8_t *buf, double *v, size_t offset)
{
	*v = nybdtol(buf, 5, offset) / 10.0;

	return v;
}

char *
ws_pressure_str(const uint8_t *buf, char *s, size_t len, size_t offset)
{
	double v;

	ws_pressure(buf, &v, offset);
	snprintf(s, len, "%.1f", v);

	return s;
}

uint8_t *
ws_humidity(const uint8_t *buf, uint8_t *v, size_t offset)
{
	*v = (uint8_t) nybdtol(buf, 2, offset);

	return v;
}

char *
ws_humidity_str(const uint8_t *buf, char *s, size_t len, size_t offset)
{
	uint8_t v;

	ws_humidity(buf, &v, offset);
	snprintf(s, len, "%hhu", v);

	return s;
}

double *
ws_rain(const uint8_t *buf, double *v, size_t offset)
{
	*v = nybdtol(buf, 6, offset) / 100.0;

	return v;
}

char *
ws_rain_str(const uint8_t *buf, char *s, size_t len, size_t offset)
{
	double v;

	ws_rain(buf, &v, offset);
	snprintf(s, len, "%.2f", v);

	return s;
}

double *
ws_speed(const uint8_t *buf, double *v, size_t offset)
{
	*v = nybtol(buf, 3, offset) / 10.0;

	return v;
}

char *
ws_speed_str(const uint8_t *buf, char *s, size_t len, size_t offset)
{
	double v;

	ws_speed(buf, &v, offset);
	snprintf(s, len, "%.1f", v);

	return s;
}

uint16_t *
ws_wind_dir(const uint8_t *buf, uint16_t *v, size_t offset)
{
	*v = nybat(buf, offset) * 22.5;

	return v;
}

char *
ws_wind_dir_str(const uint8_t *buf, char *s, size_t len, size_t offset)
{
	uint16_t v;

	ws_wind_dir(buf, &v, offset);
	snprintf(s, len, "%hu", v);

	return s;
}

double *
ws_wind_speed(const uint8_t *buf, double *v, size_t offset)
{
	*v = nybtol(buf, 2, offset) / 10.0 + nybtol(buf + 1, 2, offset) * 22.5;

	return v;
}

double *
ws_interval_sec(const uint8_t *buf, double *v, size_t offset)
{
	*v = (double) nybtol(buf, 2, offset) * 0.5;

	return v;
}

char *
ws_interval_sec_str(const uint8_t *buf, char *s, size_t len, size_t offset)
{
	double v;

	ws_interval_sec(buf, &v, offset);
	snprintf(s, len, "%.1f", v);

	return s;
}

uint16_t *
ws_interval_min(const uint8_t *buf, uint16_t *v, size_t offset)
{
	*v = (uint16_t) nybtol(buf, 3, offset);

	return v;
}

char *
ws_interval_min_str(const uint8_t *buf, char *s, size_t len, size_t offset)
{
	uint16_t v;

	ws_interval_min(buf, &v, offset);
	snprintf(s, len, "%hu", v);

	return s;
}

uint8_t *
ws_bin_2nyb(const uint8_t *buf, uint8_t *v, size_t offset)
{
	*v = (uint8_t) nybtol(buf, 2, offset);

	return v;
}

char *
ws_bin_2nyb_str(const uint8_t *buf, char *s, size_t len, size_t offset)
{
	uint8_t v;

	ws_bin_2nyb(buf, &v, offset);
	snprintf(s, len, "%hhu", v);

	return s;
}

time_t *
ws_datetime(const uint8_t *buf, time_t *v, size_t offset)
{
	struct tm tm;

	memset(&tm, 0, sizeof(tm));

	tm.tm_min = nybdtol(buf, 2, offset);
	tm.tm_hour = nybdtol(buf, 2, offset + 2);
	tm.tm_wday = nybdtol(buf, 1, offset + 4) - 1;
	tm.tm_mday = nybdtol(buf, 2, offset + 5);
	tm.tm_mon = nybdtol(buf, 2, offset + 7) - 1;
	tm.tm_year = nybdtol(buf, 2, offset + 9) + 100;
	tm.tm_isdst = -1;

	*v = mktime(&tm);

	return v;
}

char *
ws_datetime_str(const uint8_t *buf, char *s, size_t len, size_t offset)
{
	time_t ts;
	struct tm tm;

	ws_datetime(buf, &ts, offset);
	localtime_r(&ts, &tm);
	strftime(s, len, "%Y-%m-%d %H:%M", &tm);

	return s;
}

time_t *
ws_timestamp(const uint8_t *buf, time_t *v, size_t offset)
{
	struct tm tm;

	memset(&tm, 0, sizeof(tm));

	tm.tm_min = nybdtol(buf, 2, offset);
	tm.tm_hour = nybdtol(buf, 2, offset + 2);
	tm.tm_mday = nybdtol(buf, 2, offset + 4);
	tm.tm_mon = nybdtol(buf, 2, offset + 6) - 1;
	tm.tm_year = nybdtol(buf, 2, offset + 8) + 100;
	tm.tm_isdst = -1;

	*v = mktime(&tm);

	return v;
}

char *
ws_timestamp_str(const uint8_t *buf, char *s, size_t len, size_t offset)
{
	time_t ts;
	struct tm tm;

	ws_timestamp(buf, &ts, offset);
	localtime_r(&ts, &tm);
	strftime(s, len, "%Y-%m-%d %H:%M", &tm);

	return s;
}

uint8_t *
ws_connection(const uint8_t *buf, uint8_t *v, size_t offset)
{
	*v = nybat(buf, offset);

	return v;
}

uint8_t *
ws_alarm_set(const uint8_t *buf, uint8_t *v, size_t offset, uint8_t bit)
{
	*v = bit2num(buf, offset, bit);

	return v;
}

uint8_t *
ws_alarm_active(const uint8_t *buf, uint8_t *v, size_t offset, uint8_t bit)
{
	*v = bit2num(buf, offset, bit);

	return v;
}
