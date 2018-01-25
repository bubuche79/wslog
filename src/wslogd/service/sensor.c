#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <time.h>
#include <syslog.h>

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
			goto error;
		}
	} else {
		itimer_set(it, confp->driver.freq);
	}

#ifdef DEBUG
	syslog(LOG_INFO, "driver.freq: %ld\n", it->it_interval.tv_sec);
	syslog(LOG_INFO, "driver.delay: %ld\n", it->it_value.tv_sec);
#endif

	syslog(LOG_INFO, "sensor service ready");

	return 0;

error:
	return 0;
}

int
sensor_main(void)
{
	struct ws_loop buf;

	time(&buf.time);

	/* Read sensors */
	if (drv_get_loop(&buf) == -1) {
		goto error;
	}

	/* Compute derived measures */
	//	ws_calc(&buf);

	/* Push loop event in board */
	if (sensor_push(&buf) == -1) {
		syslog(LOG_ERR, "sensor_push: %m");
		goto error;
	}

#if DEBUG
	syslog(LOG_DEBUG, "Record: %.1f°C %hhu%% %.1fhPa",
			buf.temp, buf.humidity, buf.barometer);
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
