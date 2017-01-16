#include <time.h>
#include <errno.h>
#include <syslog.h>

#include "ws23xx.h"

struct ws23xx_io
{
	uint16_t addr;
	enum ws_etype type;
	void *value;
};

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
		ws_speed(buf, (float *) v, offset);
		break;
	case WS_WIND_DIR:
		ws_wind_dir(buf, (uint16_t *) v, offset);
		break;
	case WS_RAIN:
		ws23xx_rain(buf, (double *) v, offset);
		break;
	case WS_CONNECTION:
		ws_connection(buf, (uint8_t *) v, offset);
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
	printf("ws23xx_init\n");
	return 0;
}

int
ws23xx_read(struct ws_loop *p)
{
	uint8_t cnx_type;
	uint8_t cnx_countdown;
	float rain_total;
	int wind_validity;

	uint8_t buf[32];

	struct ws23xx_io io[] =
	{
			{ 0x346, WS_TEMP, buf + 0, &p->temp_in },
			{ 0x373, WS_TEMP, buf + 2, &p->temp },
			{ 0x3a0, WS_TEMP, buf + 4, &p->windchill },
			{ 0x3ce, WS_TEMP, buf + 6, &p->dew_point },
			{ 0x3fb, WS_HUMIDITY, buf + 8, &p->humidity_in },
			{ 0x419, WS_HUMIDITY, buf + 9, &p->humidity },
			{ 0x497, WS_RAIN, buf + 10, &p->rain_24h },
			{ 0x4b4, WS_RAIN, buf + 13, &p->rain_1h },
			{ 0x4d2, WS_RAIN, buf + 16, &rain_total },
			{ 0x528, WS_WIND_VALID, buf + 19, &wind_validity },
			{ 0x529, WS_SPEED, buf + 20, &p->wind_speed },
			{ 0x52c, WS_WIND_DIR, buf + 22, &p->wind_dir },
			{ 0x54d, WS_CONNECTION, buf + 23, &cnx_type },
			{ 0x54f, WS_INT_SEC, buf + 24, &cnx_countdown },
			{ 0x5d8, WS_PRESSURE, buf + 25, &p->abs_pressure },
			{ 0x5e2, WS_PRESSURE, buf + 28, &p->barometer }
	};

	size_t nel = array_len(io);

	uint16_t addr[nel];
	size_t nnyb[nel];
	uint8_t *buf[nel];

	ws_read_batch(fd, addr, nnyb, nel, buf);

	/* Decode values */
	for (size_t i = 0; i < nel; i++) {
		ws23xx_val(buf[i], io[i].type, io[i].value, 0);
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
		p->wl_mask = ~(WF_WIND_DIR|WF_WIND_SPEED);
	}

	return 0;
}

int
ws23xx_destroy(void)
{
	return 0;
}
