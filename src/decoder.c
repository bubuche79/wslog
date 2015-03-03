#include <stdio.h>
#include <time.h>
#include <string.h>

#include "decoder.h"

static uint8_t
nybble_at(const uint8_t *buf, size_t offset)
{
	return (offset & 0x1) ? buf[offset / 2] >> 4 : buf[offset / 2] & 0xF;
}

static uint64_t
bcd2num(const uint8_t *buf, uint8_t nnybble, size_t offset)
{
	int i;
	uint64_t res = 0;

	for (i = nnybble - 1; 0 <= i; i--) {
		res = 10 * res + nybble_at(buf, offset + i);
	}

	return res;
}

static uint64_t
bin2num(const uint8_t *buf, uint8_t nnybble, size_t offset)
{
	int i;
	uint64_t res = 0;

	for (i = nnybble - 1; 0 <= i; i--) {
		res = 16 * res + nybble_at(buf, offset + i);
	}

	return res;
}

static uint8_t
bit2num(const uint8_t *buf, size_t offset, uint8_t bit) {
	return nybble_at(buf, offset) & (1 << bit);
}

uint8_t
ws_nybble(const uint8_t *buf, size_t offset)
{
	return nybble_at(buf, offset);
}

double *
ws_get_temp(const uint8_t *buf, double *v, size_t offset)
{
	*v = (bcd2num(buf, 4, offset) - 3000) / 100.0;

	return v;
}

char *
ws_temp_str(const uint8_t *buf, char *s, size_t len, size_t offset)
{
	double v;

	ws_get_temp(buf, &v, offset);
	snprintf(s, len, "%.2f", v);

	return s;
}

double *
ws_get_pressure(const uint8_t *buf, double *v, size_t offset)
{
	*v = bcd2num(buf, 5, offset) / 10.0;

	return v;
}

char *
ws_pressure_str(const uint8_t *buf, char *s, size_t len, size_t offset)
{
	double v;

	ws_get_pressure(buf, &v, offset);
	snprintf(s, len, "%.1f", v);

	return s;
}

uint8_t *
ws_get_humidity(const uint8_t *buf, uint8_t *v, size_t offset)
{
	*v = (uint8_t) bcd2num(buf, 2, offset);

	return v;
}

char *
ws_humidity_str(const uint8_t *buf, char *s, size_t len, size_t offset)
{
	uint8_t v;

	ws_get_humidity(buf, &v, offset);
	snprintf(s, len, "%hhu", v);

	return s;
}

double *
ws_get_rain(const uint8_t *buf, double *v, size_t offset)
{
	*v = bcd2num(buf, 6, offset) / 100.0;

	return v;
}

char *
ws_rain_str(const uint8_t *buf, char *s, size_t len, size_t offset)
{
	double v;

	ws_get_rain(buf, &v, offset);
	snprintf(s, len, "%.2f", v);

	return s;
}

double *
ws_get_speed(const uint8_t *buf, double *v, size_t offset)
{
	*v = bin2num(buf, 3, offset) / 10.0;

	return v;
}

char *
ws_speed_str(const uint8_t *buf, char *s, size_t len, size_t offset)
{
	double v;

	ws_get_speed(buf, &v, offset);
	snprintf(s, len, "%.1f", v);

	return s;
}

double *
ws_get_wind_dir(const uint8_t *buf, double *v, size_t offset)
{
	*v = nybble_at(buf, offset) * 22.5;

	return v;
}

double *
ws_get_wind_speed(const uint8_t *buf, double *v, size_t offset)
{
	*v = bin2num(buf, 2, offset) / 10.0 + bin2num(buf + 1, 2, offset) * 22.5;

	return v;
}

uint16_t *
ws_interval_sec(const uint8_t *buf, uint16_t *v, size_t offset)
{
	*v = (uint16_t) bin2num(buf, 3, offset) * 0.5;

	return v;
}

char *
ws_interval_sec_str(const uint8_t *buf, char *s, size_t len, size_t offset)
{
	uint16_t v;

	ws_interval_min(buf, &v, offset);
	snprintf(s, len, "%hu", v);

	return s;
}

uint16_t *
ws_interval_min(const uint8_t *buf, uint16_t *v, size_t offset)
{
	*v = (uint16_t) bin2num(buf, 3, offset);

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
	*v = (uint8_t) bin2num(buf, 2, offset);

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

	tm.tm_min = bcd2num(buf, 2, offset);
	tm.tm_hour = bcd2num(buf, 2, offset + 2);
	tm.tm_wday = bcd2num(buf, 1, offset + 4) - 1;
	tm.tm_mday = bcd2num(buf, 2, offset + 5);
	tm.tm_mon = bcd2num(buf, 2, offset + 7) - 1;
	tm.tm_year = bcd2num(buf, 2, offset + 9) + 100;
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

	tm.tm_min = bcd2num(buf, 2, offset);
	tm.tm_hour = bcd2num(buf, 2, offset + 2);
	tm.tm_mday = bcd2num(buf, 2, offset + 4);
	tm.tm_mon = bcd2num(buf, 2, offset + 6) - 1;
	tm.tm_year = bcd2num(buf, 2, offset + 8) + 100;
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

char *
ws_connection_str(const uint8_t *buf, char *s, size_t len, size_t offset)
{
	uint8_t v = nybble_at(buf, offset);

	switch (v) {
	case 0:
		snprintf(s, len, "%s", "cable");
		break;
	case 3:
		snprintf(s, len, "%s", "lost");
		break;
	case 15:
		snprintf(s, len, "%s", "wireless");
		break;
	default:
		snprintf(s, len, "%x (%s)", v, "error");
		break;
	}

	return s;
}

char *
ws_alarm_set_str(const uint8_t *buf, char *s, size_t len, size_t offset, uint8_t bit)
{
	uint8_t v = bit2num(buf, offset, bit);

	if (v) {
		strncpy(s, "on", len);
	} else {
		strncpy(s, "off", len);
	}

	return s;
}

char *
ws_alarm_active_str(const uint8_t *buf, char *s, size_t len, size_t offset, uint8_t bit)
{
	uint8_t v = bit2num(buf, offset, bit);

	if (v) {
		strncpy(s, "active", len);
	} else {
		strncpy(s, "inactive", len);
	}

	return s;
}
