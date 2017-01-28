#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <time.h>
#include <syslog.h>
#ifdef DEBUG
#include <stdio.h>
#endif

#include "board.h"
#include "wslogd.h"
#include "driver/driver.h"
#include "service/util.h"
#include "service/sensor.h"

static void
sensor_derived(struct ws_loop *p)
{
//	if (sensor_updt(p, WF_WINDCHILL, WF_WIND|WF_TEMP)) {
//		p->windchill = ws_windchill(p->wind_dir, p->temp);
//	}
//	if (sensor_updt(p, WF_DEW_POINT, WF_TEMP|WF_HUMIDITY)) {
//		p->dew_point = ws_dewpoint(p->temp, p->humidity);
//	}
}

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
	printf("driver.freq: %ld\n", it->it_interval.tv_sec);
#endif

	return 0;

error:
	return 0;
}

int
sensor_main(void)
{
	int ret;
	struct ws_loop loop;

#if DEBUG
	printf("SENSOR: reading\n");
#endif

	if (clock_gettime(CLOCK_REALTIME, &loop.time) == -1) {
		goto error;
	}

	/* Read sensors */
	ret = drv_get_loop(&loop);

	if (ret == -1) {
		goto error;
	}

	/* Compute derived measures */
	sensor_derived(&loop);

	/* Push loop event in board */
	if (sensor_push(&loop) == -1) {
		goto error;
	}

#if DEBUG
	printf("SENSOR read: %.2f°C %hhu%%\n", loop.temp, loop.humidity);
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
