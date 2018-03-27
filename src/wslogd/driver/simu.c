#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>
#include <syslog.h>
#include <errno.h>

#include "libws/defs.h"
#include "libws/util.h"

#include "conf.h"
#include "driver/driver.h"
#include "driver/simu.h"

#define IO_DELAY		250
#define PI			3.1415926535897932384626433832795
#define RAD			(PI/180.0)
#define PERIOD_FACTOR		4

#define LOOP_INTERVAL		10
#define ARCHIVE_INTERVAL	300
#define ARCHIVE_DELAY		1

static int hw_archive;
static struct timespec io_delay;

static double
simu_sin(double from, double to, time_t time)
{
	double range = (to - from) / 2;

	return from + range + sin(time * RAD / PERIOD_FACTOR) * range;
}

static time_t
simu_next(time_t time, long delay)
{
	time_t r;

	r = roundup(time, delay);

	if (r == time) {
		r += delay;
	}

	return r;
}

static int
simu_io_delay()
{
	return clock_nanosleep(CLOCK_REALTIME, 0, &io_delay, NULL);
}

static void
simu_loop(struct ws_loop *p, int idx)
{
	p->barometer = simu_sin(950, 1020, idx);
	p->temp = simu_sin(15, 25, idx);
	p->humidity = simu_sin(60, 80, idx);
	p->wind_speed = simu_sin(0, 2, idx);
	p->wind_dir = simu_sin(225, 315, idx);

	/* Supported sensors */
	p->wl_mask = WF_BAROMETER|WF_TEMP|WF_HUMIDITY|WF_WIND_SPEED|WF_WIND_DIR;
}

static void
simu_archive(struct ws_archive *p, time_t time)
{
	p->time = time;
	p->barometer = simu_sin(950, 1020, time);
	p->temp = simu_sin(15, 25, time);
	p->humidity = simu_sin(60, 80, time);
	p->avg_wind_speed = simu_sin(0, 2, time);
	p->avg_wind_dir = simu_sin(225, 315, time);

	/* Supported sensors */
	p->wl_mask = WF_BAROMETER|WF_TEMP|WF_HUMIDITY|WF_WIND_SPEED|WF_WIND_DIR;
}

int
simu_init(void)
{
	long delay;

	hw_archive = confp->driver.simu.hw_archive;
	delay = confp->driver.simu.io_delay;

	if (delay== 0) {
		delay = IO_DELAY;
	}
	ws_time_ms(&io_delay, delay);

	if (simu_io_delay() == -1) {
		return -1;
	}

	syslog(LOG_NOTICE, "Simulator device initialized");

	return 0;
}

int
simu_destroy(void)
{
	if (simu_io_delay() == -1) {
		return -1;
	}

	return 0;
}

int
simu_get_itimer(struct itimerspec *it, enum ws_timer type)
{
	int ret;

	if (simu_io_delay() == -1) {
		return -1;
	}

	switch (type)
	{
	case WS_ITIMER_LOOP:
		ws_itimer_delay(it, LOOP_INTERVAL, 0);
		ret = 0;
		break;
	case WS_ITIMER_ARCHIVE:
		ws_itimer_delay(it, ARCHIVE_INTERVAL, ARCHIVE_DELAY);
		ret = 0;
		break;
	default:
		errno = EINVAL;
		ret = -1;
		break;
	}

	return ret;
}

int
simu_get_loop(struct ws_loop *p)
{
	time_t now;

	time(&now);
	simu_loop(p, now);

	return 0;
}

ssize_t
simu_get_archive(struct ws_archive *p, size_t nel, time_t after)
{
	int i;
	time_t now;
	time_t artime;

	time(&now);
	artime = simu_next(after, ARCHIVE_INTERVAL);

	for (i = 0; i < nel && artime <= now; i++) {
		simu_archive(&p[i], artime);

		artime += ARCHIVE_INTERVAL;
	}

	return i;
}
