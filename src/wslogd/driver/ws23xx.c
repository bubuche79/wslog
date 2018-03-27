#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <time.h>
#include <math.h>
#include <errno.h>
#include <pthread.h>
#include <syslog.h>

#include "libws/defs.h"
#include "libws/nybble.h"
#include "libws/serial.h"
#include "libws/ws23xx/archive.h"
#include "libws/ws23xx/decoder.h"
#include "libws/ws23xx/ws23xx.h"

#include "conf.h"
#include "ws23xx.h"

#define WF_ALL_IN	(WF_IN_TEMP|WF_IN_HUMIDITY|WF_PRESSURE|WF_BAROMETER)
#define WF_ALL_HIST	(WF_PRESSURE|WF_IN_TEMP|WF_IN_HUMIDITY|WF_TEMP|WF_HUMIDITY|WF_WIND_SPEED|WF_WIND_DIR)

struct ws23xx_io
{
	uint16_t addr;
	int type;
	size_t nnyb;
	void *p;
};

static int fd;					/* device file */
static pthread_mutex_t mutex;	/* device access mutex */
static float total_rain;		/* total rain sensor */

static void *
ws23xx_val(const uint8_t *buf, int type, void *v, size_t offset)
{
	switch (type) {
	case WS23XX_TEMP:
		ws23xx_temp(buf, (float *) v, offset);
		break;
	case WS23XX_PRESSURE:
		ws23xx_pressure(buf, (float *) v, offset);
		break;
	case WS23XX_HUMIDITY:
		ws23xx_humidity(buf, (uint8_t *) v, offset);
		break;
	case WS23XX_SPEED:
		ws23xx_speed(buf, (float *) v, offset);
		break;
	case WS23XX_WIND_DIR:
		ws23xx_wind_dir(buf, (uint16_t *) v, offset);
		break;
	case WS23XX_RAIN:
		ws23xx_rain(buf, (float *) v, offset);
		break;
	case WS23XX_CONNECTION:
		ws23xx_connection(buf, (uint8_t *) v, offset);
		break;
	case WS23XX_INT_SEC:
		ws23xx_interval_sec(buf, (float *) v, offset);
		break;
	case WS23XX_WIND_VALID:
		ws23xx_wind_valid(buf, (uint8_t *) v, offset);
		break;
	case WS23XX_WIND_OVERFLOW:
		ws23xx_wind_overflow(buf, (uint8_t *) v, offset);
		break;
	default:
		v = NULL;
		errno = ENOTSUP;
		break;
	}

	return v;
}

/**
 * Check hardware limits.
 *
 * This function clears flags of structure pointed to by {@code p} that are
 * out of range, based on hardware specification.
 */
static void
ws23xx_hw_limits(struct ws_loop *p, int log)
{
	uint32_t prev_mask;

	prev_mask = p->wl_mask;

	/* Outdoor sensors */
	if (p->temp < -29.9 || 69.9 < p->temp) {
		p->wl_mask &= ~WF_TEMP;
	}
	if (p->humidity < 10 || 99 < p->humidity) {
		p->wl_mask &= ~WF_HUMIDITY;
	}
	if (p->wind_speed < 0.0 || 50 < p->wind_speed) {
		p->wl_mask &= ~WF_WIND_SPEED;
	}

	/* Indoor sensors */
	if (p->in_temp < -9.9 || 59.9 < p->temp) {
		p->wl_mask &= ~WF_IN_TEMP;
	}
	if (p->humidity < 1 || 99 < p->humidity) {
		p->wl_mask &= ~WF_IN_HUMIDITY;
	}
//	if (p->pressure < 760 || 1099 < p->pressure) {
//		p->wl_mask &= ~WF_PRESSURE;
//	}

	/* Compare against previous mask */
	if (log && prev_mask != p->wl_mask) {
		uint32_t diff_mask = prev_mask ^ p->wl_mask;

		syslog(LOG_WARNING, "Unexpected sensor values (%x mask)", diff_mask);
	}
}

int
ws23xx_init(void)
{
	total_rain = -1;

	fd = ws23xx_open(confp->driver.ws23xx.tty);
	if (fd == -1) {
		return -1;
	}

	if (pthread_mutex_init(&mutex, NULL) == -1) {
		return -1;
	}

	return 0;
}

int
ws23xx_destroy(void)
{
	int ret;

	ret = pthread_mutex_destroy(&mutex);

	if (ws_close(fd) == -1) {
		ret = -1;
	}

	return ret;
}

int
ws23xx_get_itimer(struct itimerspec *it, enum ws_timer type)
{
	uint8_t buf[8];

	it->it_interval.tv_nsec = 0;
	it->it_value.tv_nsec = 0;

	if (pthread_mutex_lock(&mutex) == -1) {
		return -1;
	}

	if (WS_ITIMER_LOOP == type) {
		uint8_t cnx_type;
		float cnx_countdown;

		if (ws23xx_read_safe(fd, 0x54d, 4, buf) == -1) {
			goto error;
		}

		ws23xx_connection(buf, &cnx_type, 0);
		ws23xx_interval_sec(buf, &cnx_countdown, 2);

		switch (cnx_type) {
		case 0:				/* cable */
			it->it_interval.tv_sec = 8;
			it->it_value.tv_sec = lround(cnx_countdown) + 2;
			break;

		case 15:			/* wireless */
			it->it_interval.tv_sec = 128;
			it->it_value.tv_sec = lround(cnx_countdown) + 10;
			break;

		default:
			goto error;
			break;
		}
	} else if (WS_ITIMER_ARCHIVE == type) {
		uint16_t save_int;
		time_t last_sample;

		if (ws23xx_read_safe(fd, 0x6b2, 16, buf) == -1) {
			goto error;
		}

		ws23xx_interval_min(buf, &save_int, 0);
		ws23xx_timestamp(buf, &last_sample, 6);

		it->it_interval.tv_sec = (save_int + 1) * 60;
		it->it_value.tv_sec = last_sample + it->it_interval.tv_sec - time(NULL) + 10;
	} else {
		errno = EINVAL;
		goto error;
	}

	if (pthread_mutex_unlock(&mutex) == -1) {
		return -1;
	}

	return 0;

error:
	(void) pthread_mutex_unlock(&mutex);
	return -1;
}

int
ws23xx_get_loop(struct ws_loop *p)
{
	uint8_t cnx_type;
	uint8_t wind_invalid;
	uint8_t wind_overflow;
	float total_rain_now;

	uint8_t abuf[64];

	struct ws23xx_io io[] =
	{
		{ 0x346, WS23XX_TEMP, 4, &p->in_temp },
		{ 0x373, WS23XX_TEMP, 4, &p->temp },
		{ 0x3a0, WS23XX_TEMP, 4, &p->windchill },
		{ 0x3ce, WS23XX_TEMP, 4, &p->dew_point },
		{ 0x3fb, WS23XX_HUMIDITY, 2, &p->in_humidity },
		{ 0x419, WS23XX_HUMIDITY, 2, &p->humidity },
#if 0
		{ 0x497, WS23XX_RAIN, 6, &p->rain_24h },
		{ 0x4b4, WS23XX_RAIN, 6, &p->rain_1h },
#endif
		{ 0x4d2, WS23XX_RAIN, 6, &total_rain_now },
		{ 0x527, WS23XX_WIND_OVERFLOW, 1, &wind_overflow },
		{ 0x528, WS23XX_WIND_VALID, 1, &wind_invalid },
		{ 0x529, WS23XX_SPEED, 3, &p->wind_speed },
		{ 0x52c, WS23XX_WIND_DIR, 1, &p->wind_dir },
		{ 0x54d, WS23XX_CONNECTION, 1, &cnx_type },
//		{ 0x5d8, WS23XX_PRESSURE, 5, &p->pressure },
		{ 0x5e2, WS23XX_PRESSURE, 5, &p->barometer }
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
	if (pthread_mutex_lock(&mutex) == -1) {
		return -1;
	}

	if (ws23xx_read_batch(fd, addr, nnyb, nel, buf) == -1) {
		goto error;
	}

	if (pthread_mutex_unlock(&mutex) == -1) {
		return -1;
	}

	/* Decode values */
	p->wl_mask = WF_ALL_IN;

	for (size_t i = 0; i < nel; i++) {
		ws23xx_val(buf[i], io[i].type, io[i].p, 0);
	}

	/* Clear some fields */
	switch (cnx_type) {
	case 0:				/* cable */
	case 15:			/* wireless */
		p->wl_mask |= WF_TEMP|WF_HUMIDITY|WF_RAIN|WF_DEW_POINT;

		if (!(wind_invalid || wind_overflow)) {
			p->wl_mask |= WF_WIND_SPEED|WF_WIND_DIR|WF_WINDCHILL;
		}

		/* Skip first value */
		if (total_rain != -1.0) {
//			p->rain = total_rain_now - total_rain;
		}
		total_rain = total_rain_now;
		break;

	default:
		syslog(LOG_WARNING, "Connection: %x lost", cnx_type);
		break;
	}

	ws23xx_hw_limits(p, 1);

	return 0;

error:
	(void) pthread_mutex_unlock(&mutex);
	return -1;
}

// TODO: handle after argument
ssize_t
ws23xx_get_archive(struct ws_archive *ar, size_t nel, time_t after)
{
	ssize_t i, res;
	struct ws23xx_ar arbuf[nel];

	/* Read from device */
	if (pthread_mutex_lock(&mutex) == -1) {
		return -1;
	}

	res = ws23xx_fetch_ar(fd, arbuf, nel);
	if (res <= 0) {
		goto error;
	}

	if (pthread_mutex_unlock(&mutex) == -1) {
		return -1;
	}

	/* Copy values */
	for (i = 0; i < res; i++) {
		ar[i].time = arbuf[i].tstamp;

		ar[i].wl_mask = WF_ALL_HIST;
#if 0
		ar[i].pressure = arbuf[i].pressure;
#endif
		ar[i].in_temp = arbuf[i].temp_in;
		ar[i].in_humidity = arbuf[i].humidity_in;
		ar[i].temp = arbuf[i].temp;
		ar[i].humidity = arbuf[i].humidity;
		ar[i].avg_wind_speed = arbuf[i].wind_speed;
		ar[i].avg_wind_dir = arbuf[i].wind_dir;

		// TODO
//		ws23xx_hw_limits(&ar[i], 0);

		/* History limits (seems to differ from hardware limits) */
		if (69.7 < ar[i].temp) {
			ar[i].wl_mask &= ~WF_TEMP;
		}
	}

	return res;

error:
	(void) pthread_mutex_unlock(&mutex);
	return -1;
}

int
ws23xx_set_artimer(long itmin, long next)
{
	uint8_t buf[3];

	if ((itmin < 1 && 0xFFF < itmin) || 0xFFF < next) {
		errno = EINVAL;
		return -1;
	}

	itmin--;
	if (next >= 1) {
		next--;
	}

	if (pthread_mutex_lock(&mutex) == -1) {
		return -1;
	}

	ultonyb(buf, 3, 0, itmin, 16);			/* history interval */
	ultonyb(buf, 3, 3, next, 16);			/* history time till sample */

	if (ws23xx_write_safe(fd, 0x6b2, 6, WRITENIB, buf) == -1) {
		goto error;
	}

	ultonyb(buf, 2, 0, 0, 16);				/* history number of records */

	if (ws23xx_write_safe(fd, 0x6c4, 2, WRITENIB, buf) == -1) {
		goto error;
	}

	if (pthread_mutex_unlock(&mutex) == -1) {
		return -1;
	}

	return 0;

error:
	(void) pthread_mutex_unlock(&mutex);
	return -1;
}
