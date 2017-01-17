#include <time.h>
#include <errno.h>
#include <syslog.h>

#include "defs/std.h"
#include "libws/ws23xx/decoder.h"
#include "libws/ws23xx/ws23xx.h"

#include "ws23xx.h"

static void *
ws23xx_val(const uint8_t *buf, enum ws_etype type, void *v, size_t offset)
{
	switch (type) {
	case WS_TEMP:
		ws23xx_temp(buf, (float *) v, offset);
		break;
	case WS_PRESSURE:
		ws23xx_pressure(buf, (float *) v, offset);
		break;
	case WS_HUMIDITY:
		ws23xx_humidity(buf, (uint8_t *) v, offset);
		break;
	case WS_SPEED:
		ws23xx_speed(buf, (float *) v, offset);
		break;
	case WS_WIND_DIR:
		ws23xx_wind_dir(buf, (uint16_t *) v, offset);
		break;
	case WS_RAIN:
		ws23xx_rain(buf, (float *) v, offset);
		break;
	case WS_CONNECTION:
		ws23xx_connection(buf, (uint8_t *) v, offset);
		break;
	default:
		v = NULL;
		errno = ENOTSUP;
		break;
	}

	return v;
}

int
ws23xx_init(void)
{
	return 0;
}

int
ws23xx_fetch(struct ws_loop *p)
{
	uint8_t cnx_type;
	uint8_t cnx_countdown;
	float rain_total;
	int wind_validity;

	uint8_t abuf[32];

	struct ws23xx_io
	{
		uint16_t addr;
		enum ws_etype type;
		size_t nnyb;
		void *p;
	};

	struct ws23xx_io io[] =
	{
			{ 0x346, WS_TEMP, 4, &p->temp_in },
			{ 0x373, WS_TEMP, 4, &p->temp },
			{ 0x3a0, WS_TEMP, 4, &p->windchill },
			{ 0x3ce, WS_TEMP, 4, &p->dew_point },
			{ 0x3fb, WS_HUMIDITY, 2, &p->humidity_in },
			{ 0x419, WS_HUMIDITY, 2, &p->humidity },
			{ 0x497, WS_RAIN, 6, &p->rain_24h },
			{ 0x4b4, WS_RAIN, 6, &p->rain_1h },
			{ 0x4d2, WS_RAIN, 6, &rain_total },
			{ 0x527, WS_WIND_OVERFLOW, 1, &wind_validity },
			{ 0x528, WS_WIND_VALID, 1, &wind_validity },
			{ 0x529, WS_SPEED, 3, &p->wind_speed },
			{ 0x52c, WS_WIND_DIR, 1, &p->wind_dir },
			{ 0x54d, WS_CONNECTION, 1, &cnx_type },
			{ 0x54f, WS_INT_SEC, 2, &cnx_countdown },
			{ 0x5d8, WS_PRESSURE, 5, &p->abs_pressure },
			{ 0x5e2, WS_PRESSURE, 5, &p->barometer }
	};

	size_t nel = array_size(io);

	size_t off;
	uint16_t addr[nel];
	size_t nnyb[nel];
	uint8_t *buf[nel];

	off = 0;

	for (size_t i = 0; i < nel; i++) {
		addr[i] = io[i].addr;
		nnyb[i] = io[i].nnyb;
		buf[i] = abuf + off;

		off += divup(io[i].nnyb, 2);
	}

	/* Read from device */
	if (ws23xx_read_batch(0, addr, nnyb, nel, buf) == -1) {
		return -1;
	}

	/* Decode values */
	for (size_t i = 0; i < nel; i++) {
		ws23xx_val(buf[i], io[i].type, io[i].p, 0);
	}

	/* Clear some fields */
	switch (cnx_type) {
	case 0:				/* cable */
	case 15:			/* wireless */
		p->wl_mask = WF_ALL;
		break;

	default:
		p->wl_mask = WF_TEMP_IN|WF_HUMIDITY_IN|WF_PRESSURE;
		syslog(LOG_WARNING, "Connection: %x lost", cnx_type);
		break;
	}

	if (!wind_validity) {
		p->wl_mask = ~WF_WIND;
	}

	return 0;
}

int
ws23xx_destroy(void)
{
	return 0;
}
