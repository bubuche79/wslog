#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <time.h>
#include <syslog.h>
#ifdef DEBUG
#include <stdio.h>
#endif

#include "board.h"
#include "conf.h"
#include "driver/driver.h"
#include "service/util.h"
#include "service/sensor.h"

static int
sensor_push(const struct ws_loop *p)
{
	if (board_lock() == -1) {
		return -1;
	}

	board_push(p);

	if (board_unlock() == -1) {
		return -1;
	}

	return 0;
}

int
sensor_init(struct itimerspec *it)
{
	/*
	 * Use configuration supplied frequency. When not set, use the station
	 * sensor time otherwise, when supported by the driver.
	 */
	if (confp->driver.freq == 0) {
		if (drv_get_itimer(it, WS_ITIMER_LOOP) == -1) {
			syslog(LOG_ERR, "drv_get_itimer: %m");
			goto error;
		}
	} else {
		itimer_set(it, confp->driver.freq);
	}

#ifdef DEBUG
	printf("driver.freq: %ld\n", it->it_interval.tv_sec);
	printf("driver.delay: %ld\n", it->it_value.tv_sec);
#endif

	syslog(LOG_INFO, "%s: done", __func__);

	return 0;

error:
	return 0;
}

int
sensor_main(void)
{
	struct ws_loop buf;

#if DEBUG
	printf("SENSOR: reading\n");
#endif

	time(&buf.time);

	/* Read sensors */
	if (drv_get_loop(&buf) == -1) {
		syslog(LOG_ERR, "clock_gettime: %m");
		goto error;
	}

	/* Compute derived measures */
	ws_calc(&buf);

	/* Push loop event in board */
	if (sensor_push(&buf) == -1) {
		syslog(LOG_ERR, "sensor_push: %m");
		goto error;
	}

#if DEBUG
	printf("SENSOR read: %.2f°C %hhu%%\n", buf.temp, buf.humidity);
#endif

	return 0;

error:
	return -1;
}

int
sensor_destroy(void)
{
	return 0;
}