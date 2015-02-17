#include <stdio.h>
#include <math.h>

#include "ws2300.h"

static const struct conv conv[] =
{
		/* BCD number converters */
		{ "C", 4, "temperature", 2, .offset = -3000 },
		{ "hPa", 5, "pressure", 1 },
		{ "%", 2, "humidity" },
		{ "mm", 6, "rain", 2 },
		{ "m/s", 3, "speed", 1 },

		/* Wind direction converter */
		{ "°", 1, "wind direction, north=0 clockwise" },

		/* Wind velocity converter */
		{ "ms,d", 4, "wind speed and direction" },

		/* Bin converters */
		{ "s", 2, "time interval", .multi = 5 },
		{ "min", 3, "time interval", .multi = 1 },
};

const struct conv *ws_conv_temp = &conv[0];

static uint64_t
bcd2num(const uint8_t *buf, size_t n)
{
	long res = 0;

	for (int i = n - 1; 0 <= i; i--) {
		res = 10 * res + nybble_at(buf, i);
	}

	return res;
}

static uint64_t
bin2num(const uint8_t *buf, size_t n)
{
	long res = 0;

	for (int i = n - 1; 0 <= i; i--) {
		res = 16 * res + nybble_at(buf, i);
	}

	return res;
}

static double
bcd_conv(const uint8_t *buf, const struct conv *c)
{
	return (bcd2num(buf, c->nybble_count) + c->offset) / pow(10.0, c->scale);
}

static void
bcd_conv_str(const uint8_t *buf, const struct conv *c, char *str, size_t len)
{
	snprintf(str, len, "%.*f", c->scale, bcd_conv(buf, c));
}

static double
bin_conv(const uint8_t *buf, const struct conv *c)
{
	return bin2num(buf, c->nybble_count) * c->multi / pow(10.0, c->scale);
}

double
ws_get_temp(const uint8_t *buf)
{
	return bcd_conv(buf, ws_conv_temp);
}

void
ws_get_temp_str(const uint8_t *buf, char *str, size_t len)
{
	return bcd_conv_str(buf, ws_conv_temp, str, len);
}

double
ws_get_pressure(const uint8_t *buf)
{
	return bcd_conv(buf, &conv[1]);
}

double
ws_get_humidity(const uint8_t *buf)
{
	return bcd_conv(buf, &conv[2]);
}

double
ws_get_rain(const uint8_t *buf)
{
	return bcd_conv(buf, &conv[3]);
}

double
ws_get_speed(const uint8_t *buf)
{
	return bcd_conv(buf, &conv[4]);
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
