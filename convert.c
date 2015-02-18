#include <stdio.h>
#include <math.h>

#include "convert.h"

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

		/* Date and time converters */
		{ NULL, 6, "yyyy-mm-dd", .tm = { "%Y-%m-%d" } },
		{ NULL, 10, "yyy-mm-dd hh:mm", .tm = { "%Y-%m-%d %H:%M" } },
		{ NULL, 11, "yyy-mm-dd hh:mm", .tm = { "%Y-%m-%d %H:%M" } },
		{ NULL, 6, "hh:mm:ss", .tm = { "%H:%M:%S" } },
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
	long res = 0;

	for (int i = n - 1; 0 <= i; i--) {
		res = 10 * res + nybble_at(buf, i);
	}

	return res;
}

static long
bin2num(const uint8_t *buf, size_t n)
{
	long res = 0;

	for (int i = n - 1; 0 <= i; i--) {
		res = 16 * res + nybble_at(buf, i);
	}

	return res;
}

static double
bcd_conv(const uint8_t *buf, const struct ws_conv *c)
{
	return (bcd2num(buf, c->nybbles) + c->bcd.offset) / pow(10.0, c->bcd.scale);
}

static void
bcd_conv_str(const uint8_t *buf, const struct ws_conv *c, char *str, size_t len)
{
	snprintf(str, len, "%.*f %s", c->bcd.scale, bcd_conv(buf, c), c->units);
}

static double
bin_conv(const uint8_t *buf, const struct ws_conv *c)
{
	return bin2num(buf, c->nybbles) * c->bin.multi / pow(10.0, c->bin.scale);
}

static void
bin_conv_str(const uint8_t *buf, const struct ws_conv *c, char *str, size_t len)
{
	snprintf(str, len, "%.*f %s", c->bcd.scale, bin_conv(buf, c), c->units);
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

void
ws_temp_str(const uint8_t *buf, char *str, size_t len)
{
	return bcd_conv_str(buf, get_conv(WS_TEMP), str, len);
}

double
ws_get_pressure(const uint8_t *buf)
{
	return bcd_conv(buf, get_conv(WS_PRESSURE));
}

void
ws_pressure_str(const uint8_t *buf, char *str, size_t len)
{
	return bcd_conv_str(buf, get_conv(WS_PRESSURE), str, len);
}

double
ws_get_humidity(const uint8_t *buf)
{
	return bcd_conv(buf, get_conv(WS_HUMIDITY));
}

void
ws_humidity_str(const uint8_t *buf, char *str, size_t len)
{
	return bcd_conv_str(buf, get_conv(WS_HUMIDITY), str, len);
}

double
ws_get_rain(const uint8_t *buf)
{
	return bcd_conv(buf, get_conv(WS_RAIN));
}

void
ws_rain_str(const uint8_t *buf, char *str, size_t len)
{
	return bcd_conv_str(buf, get_conv(WS_RAIN), str, len);
}

double
ws_get_speed(const uint8_t *buf)
{
	return bcd_conv(buf, get_conv(WS_SPEED));
}

void
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
