#include <math.h>

#include "ws2300.h"

struct conv
{
	char units[4];				/* units name (eg hPa) */
	uint8_t nybble_count;		/* nybbles count */
	char descr[64];				/* units description */
	uint8_t scale;				/* value scale */

	union {
		struct {
			int offset;			/* value offset */
		};
		struct {
			int multi;			/* multiplicity factor */
		};
	};
};

const struct conv conv[] =
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

		/* Bit converters */
};

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

static double
bin_conv(const uint8_t *buf, const struct conv *c)
{
	return bin2num(buf, c->nybble_count) * c->multi / pow(10.0, c->scale);
}

double
ws_get_temp(const uint8_t *buf)
{
	return bcd_conv(buf, &conv[0]);
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
