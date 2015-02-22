#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>

#include "decoder.h"

static const struct ws_conv ws_conv[] =
{
		/* BCD number converters */
		{ "°C", 4, "temperature", .bcd = { 2, -3000 } },
		{ "hPa", 5, "pressure", .bcd = { 1, 0 } },
		{ "%", 2, "humidity" },
		{ "mm", 6, "rain", .bcd = { 2, 0 } },
		{ "m/s", 3, "speed", .bcd = { 1, 0 } },

		/* Wind direction converter */
		{ "deg", 1, "wind direction, North=0 clockwise" },

		/* Wind velocity converter */
		{ "ms,d", 4, "wind speed and direction" },

		/* Bin converters */
		{ "s", 2, "time interval", .bin = { 0, 5 } },
		{ "min", 3, "time interval", .bin = { 0, 1 } },
		{ NULL, 2, "record number", .bin = { 0, 1 } },

		/* Date and time converters */
		{ NULL, 6, "yyyy-mm-dd", .tm = { "%Y-%m-%d" } },
		{ NULL, 10, "yyy-mm-dd hh:mm", .tm = { "%Y-%m-%d %H:%M" } },
		{ NULL, 11, "yyy-mm-dd hh:mm", .tm = { "%Y-%m-%d %H:%M" } },
		{ NULL, 6, "hh:mm:ss", .tm = { "%H:%M:%S" } },

		/* Text converters */
		{ NULL, 1, "cable, lost, wireless", .text.map = { { 0, "cable" }, { 3, "lost" }, { 15, "wireless" } } },
		{ NULL, 1, "rainy, cloudy, sunny", .text.map = { { 0, "rainy" }, { 1, "cloudy" }, { 2, "sunny" } } },
		{ NULL, 1, "rainy, cloudy, sunny", .text.map = { { 0, "m/s" }, { 1, "knots" }, { 2, "beaufort" }, { 3, "km/h" }, { 4, "mph" } } },
};

static inline uint8_t
nybble_at(const uint8_t *buf, size_t i)
{
	return (i & 0x1) ? buf[i / 2] >> 4 : buf[i / 2] & 0xF;

}

static inline const struct ws_conv *
get_conv(enum ws_type t)
{
	return &ws_conv[t];
}

static long
bcd2num(const uint8_t *buf, size_t n)
{
	int i;
	long res = 0;

	for (i = n - 1; 0 <= i; i--) {
		res = 10 * res + nybble_at(buf, i);
	}

	return res;
}

static long
bin2num(const uint8_t *buf, size_t n)
{
	int i;
	long res = 0;

	for (i = n - 1; 0 <= i; i--) {
		res = 16 * res + nybble_at(buf, i);
	}

	return res;
}

static double
bcd_conv(const uint8_t *buf, const struct ws_conv *c)
{
	return (bcd2num(buf, c->nybble) + c->bcd.offset) / pow(10.0, c->bcd.scale);
}

static char *
bcd_conv_str(const uint8_t *buf, const struct ws_conv *c, char *str, size_t len)
{
	snprintf(str, len, "%.*f", c->bcd.scale, bcd_conv(buf, c));

	return str;
}

static double
bin_conv(const uint8_t *buf, const struct ws_conv *c)
{
	return bin2num(buf, c->nybble) * c->bin.multi / pow(10.0, c->bin.scale);
}

static char *
bin_conv_str(const uint8_t *buf, const struct ws_conv *c, char *str, size_t len)
{
	snprintf(str, len, "%.*f", c->bcd.scale, bin_conv(buf, c));

	return str;
}

static char *
map_conv_str(const uint8_t *buf, const struct ws_conv *c, char *str, size_t len)
{
	char *result = NULL;
	uint8_t key = nybble_at(buf, 0);

	for (int i = 0; result != NULL && i < 3; i++) {
		if (c->text.map[i].key == key) {
			result = c->text.map[i].value;
		}
	}

	snprintf(str, len, "%s", result);

	return str;
}

const struct ws_conv *
ws_get_conv(enum ws_type t)
{
	return get_conv(t);
}

double
ws_get_temp(const uint8_t *buf)
{
	return bcd_conv(buf, get_conv(WS_TEMP));
}

char *
ws_temp_str(const uint8_t *buf, char *str, size_t len)
{
	return bcd_conv_str(buf, get_conv(WS_TEMP), str, len);
}

double
ws_get_pressure(const uint8_t *buf)
{
	return bcd_conv(buf, get_conv(WS_PRESSURE));
}

char *
ws_pressure_str(const uint8_t *buf, char *str, size_t len)
{
	return bcd_conv_str(buf, get_conv(WS_PRESSURE), str, len);
}

double
ws_get_humidity(const uint8_t *buf)
{
	return bcd_conv(buf, get_conv(WS_HUMIDITY));
}

char *
ws_humidity_str(const uint8_t *buf, char *str, size_t len)
{
	return bcd_conv_str(buf, get_conv(WS_HUMIDITY), str, len);
}

double
ws_get_rain(const uint8_t *buf)
{
	return bcd_conv(buf, get_conv(WS_RAIN));
}

char *
ws_rain_str(const uint8_t *buf, char *str, size_t len)
{
	return bcd_conv_str(buf, get_conv(WS_RAIN), str, len);
}

double
ws_get_speed(const uint8_t *buf)
{
	return bcd_conv(buf, get_conv(WS_SPEED));
}

char *
ws_speed_str(const uint8_t *buf, char *str, size_t len)
{
	return bcd_conv_str(buf, get_conv(WS_SPEED), str, len);
}

double
ws_get_wind_dir(const uint8_t *buf)
{
	return buf[0] * 22.5;
}

double
ws_get_wind_speed(const uint8_t *buf)
{
	return bcd2num(buf, 2) / 10.0 + bin2num(buf + 1, 2) * 22.5;
}

double
ws_get_interval_sec(const uint8_t *buf)
{
	return bin_conv(buf, get_conv(WS_INT_SEC));
}

char *
ws_interval_sec_str(const uint8_t *buf, char *str, size_t len)
{
	return bin_conv_str(buf, get_conv(WS_INT_SEC), str, len);
}

double
ws_get_interval_min(const uint8_t *buf)
{
	return bin_conv(buf, get_conv(WS_INT_MIN));
}

char *
ws_interval_min_str(const uint8_t *buf, char *str, size_t len)
{
	return bin_conv_str(buf, get_conv(WS_INT_MIN), str, len);
}

double
ws_get_2nyb_min(const uint8_t *buf)
{
	return bin_conv(buf, get_conv(WS_BIN_2NYB));
}

char *
ws_2nyb_str(const uint8_t *buf, char *str, size_t len)
{
	return bin_conv_str(buf, get_conv(WS_BIN_2NYB), str, len);
}

time_t
ws_get_datetime(const uint8_t *buf)
{
	struct tm tm;

	memset(&tm, 0, sizeof(tm));

	tm.tm_min = ((buf[0] >> 4) * 10) + (buf[0] & 0xF);
	tm.tm_hour = ((buf[1] >> 4) * 10) + (buf[1] & 0xF);
	tm.tm_wday = (buf[2] & 0xF) - 1;
	tm.tm_mday = ((buf[3] & 0xF) * 10) + (buf[2] >> 4);
	tm.tm_mon = ((buf[4] & 0xF) * 10) + (buf[3] >> 4) - 1;
	tm.tm_year = 100 + ((buf[5] & 0xF) * 10) + (buf[4] >> 4);
	tm.tm_isdst = -1;

	return mktime(&tm);
}

char *
ws_datetime_str(const uint8_t *buf, char *str, size_t len)
{
	time_t ts;
	struct tm tm;

	ts = ws_get_datetime(buf);
	localtime_r(&ts, &tm);
	strftime(str, len, get_conv(WS_DATETIME)->tm.format, &tm);

	return str;
}

time_t
ws_get_timestamp(const uint8_t *buf)
{
	struct tm tm;

	memset(&tm, 0, sizeof(tm));

	tm.tm_min = ((buf[0] >> 4) * 10) + (buf[0] & 0xF);
	tm.tm_hour = ((buf[1] >> 4) * 10) + (buf[1] & 0xF);
	tm.tm_mday = ((buf[2] >> 4) * 10) + (buf[2] & 0xF);
	tm.tm_mon = ((buf[3] >> 4) * 10) + (buf[3] & 0xF) - 1;
	tm.tm_year = 100 + ((buf[4] >> 4) * 10) + (buf[4] & 0xF);
	tm.tm_isdst = -1;

	return mktime(&tm);
}

char *
ws_timestamp_str(const uint8_t *buf, char *str, size_t len)
{
	time_t ts;
	struct tm tm;

	ts = ws_get_timestamp(buf);
	localtime_r(&ts, &tm);
	strftime(str, len, get_conv(WS_TIMESTAMP)->tm.format, &tm);

	return str;
}

char *
ws_connection_str(const uint8_t *buf, char *str, size_t len)
{
	return map_conv_str(buf, get_conv(WS_CONNECTION), str, len);
}

void
ws_decode_history(const uint8_t *buf, struct ws_history *h)
{
	long v;

	v = ((buf[2] & 0xF) << 16) + (buf[1] << 8) + buf[0];
	h->temp_in = (v % 1000) / 10.0 - 30.0;
	h->temp_out = (v - (v % 1000)) / 10000.0 - 30.0;

	v = (buf[4] << 12) + (buf[3] <<4 ) + (buf[2] >> 4);
	h->abs_pressure = (v % 10000) / 10.0;
	if (h->abs_pressure < 502.2) {
		h->abs_pressure += 1000;
	}
	h->humidity_in = (v - (v % 10000)) / 10000.0;

	h->humidity_out = (buf[5] >> 4) * 10 + (buf[5] & 0xF);
	h->rain = ((buf[7] & 0xF) * 256 + buf[6]) * 0.518;
	h->wind_speed = (buf[8] * 16 + (buf[7] >> 4))/ 10.0;
	h->wind_dir = (buf[9] & 0xF) * 22.5;
}
