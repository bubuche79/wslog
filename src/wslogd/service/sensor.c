#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <time.h>
#include <syslog.h>

#include "conf.h"
#include "driver/driver.h"
#include "service/util.h"
#include "service/sensor.h"

int
sensor_init(struct itimerspec *it)
{
	/*
	 * Use configuration supplied frequency. When not set, use the station
	 * sensor time otherwise, when supported by the driver.
	 */
	if (confp->driver.freq == 0) {
		if (drv_get_rt_itimer(it) == -1) {
			goto error;
		}
	} else {
		it->it_interval.tv_sec = confp->driver.freq / 1000;
		it->it_interval.tv_nsec = confp->driver.freq - it->it_interval.tv_sec;
		it->it_value.tv_sec = 0;
		it->it_value.tv_nsec = 0;
	}

#ifdef DEBUG
	syslog(LOG_INFO, "driver.freq: %ld\n", it->it_interval.tv_sec);
	syslog(LOG_INFO, "driver.delay: %ld\n", it->it_value.tv_sec);
#endif

	syslog(LOG_INFO, "Sensor service ready");

	return 0;

error:
	return -1;
}

int
sensor_sig_timer(struct ws_loop *rt)
{
	rt->time.tv_sec = 0;

	/* Read sensors */
	if (drv_get_rt(rt) == -1) {
		goto error;
	}

	if (rt->time.tv_sec == 0) {
		if (clock_gettime(CLOCK_REALTIME, &rt->time) == -1) {
			syslog(LOG_ERR, "clock_gettime(): %m");
			goto error;
		}
	}

#if DEBUG
	syslog(LOG_DEBUG, "Sensor: %.1f°C %hhu%% %.1fhPa",
			rt->temp, rt->humidity, rt->barometer);
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
