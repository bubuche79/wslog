
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>
#include <syslog.h>
#include <errno.h>

#include "libws/defs.h"

#include "conf.h"
#include "service/util.h"
#include "driver/driver.h"
#include "driver/virt.h"

#define IO_DELAY		250
#define PI			3.1415926535897932384626433832795
#define RAD			(PI/180.0)
#define PERIOD_FACTOR		4

#define LOOP_INTERVAL		2
#define ARCHIVE_INTERVAL	60
#define ARCHIVE_DELAY		5

static int hw_archive;
static struct timespec io_delay;

static double
virt_sin(double from, double to, time_t time)
{
	double range = (to - from) / 2;

	return from + range + sin(time * RAD / PERIOD_FACTOR) * range;
}

static time_t
virt_next(time_t time, long delay)
{
	time_t r;

	r = roundup(time, delay);

	if (r == time) {
		r += delay;
	}

	return r;
}

static int
virt_io_delay()
{
	return clock_nanosleep(CLOCK_REALTIME, 0, &io_delay, NULL);
}

static void
virt_loop(struct ws_loop *p, time_t time)
{
	p->barometer = virt_sin(950, 1020, time);
	p->temp = virt_sin(15, 25, time);
	p->humidity = virt_sin(60, 80, time);
	p->wind_speed = virt_sin(0, 2, time);
	p->wind_dir = virt_sin(225, 315, time);

	/* Supported sensors */
	p->wl_mask = WF_BAROMETER|WF_TEMP|WF_HUMIDITY|WF_WIND_SPEED|WF_WIND_DIR;
}

static void
virt_archive(struct ws_archive *p, time_t time)
{
	p->time = time;
	p->barometer = virt_sin(950, 1020, time);
	p->temp = virt_sin(15, 25, time);
	p->humidity = virt_sin(60, 80, time);
	p->avg_wind_speed = virt_sin(0, 2, time);
	p->avg_wind_dir = virt_sin(225, 315, time);

	/* Supported sensors */
	p->wl_mask = WF_BAROMETER|WF_TEMP|WF_HUMIDITY|WF_WIND_SPEED|WF_WIND_DIR;
}

int
virt_init(void)
{
	long delay;

	hw_archive = confp->driver.virt.hw_archive;
	delay = confp->driver.virt.io_delay;

	if (delay == 0) {
		delay = IO_DELAY;
	}

	io_delay.tv_sec = delay / 1000 ;
	io_delay.tv_nsec = delay * 1000000 - io_delay.tv_sec;

	if (virt_io_delay() == -1) {
		return -1;
	}

	syslog(LOG_NOTICE, "Simulator device initialized");

	return 0;
}

int
virt_destroy(void)
{
	if (virt_io_delay() == -1) {
		return -1;
	}

	return 0;
}

int
virt_get_rt(struct ws_loop *p)
{
	time_t now;

	time(&now);
	virt_loop(p, now);

	return 0;
}

int
virt_get_rt_itimer(struct itimerspec *it)
{
	if (virt_io_delay() == -1) {
		return -1;
	}

	itimer_setdelay(it, LOOP_INTERVAL, 0);

	return 0;
}

ssize_t
virt_get_ar(struct ws_archive *p, size_t nel, time_t after)
{
	int i;
	time_t now;
	time_t artime;

	time(&now);
	artime = virt_next(after, ARCHIVE_INTERVAL);

	for (i = 0; i < nel && artime <= now; i++) {
		virt_archive(&p[i], artime);

		artime += ARCHIVE_INTERVAL;
	}

	return i;
}

int
virt_get_ar_itimer(struct itimerspec *it)
{
	if (virt_io_delay() == -1) {
		return -1;
	}

	itimer_setdelay(it, ARCHIVE_INTERVAL, ARCHIVE_DELAY);

	return 0;
}

int
virt_time(time_t *t)
{
	if (virt_io_delay() == -1) {
		return -1;
	}

	time(t);

	return 0;
}

int
virt_adjtime(time_t t)
{
	if (virt_io_delay() == -1) {
		return -1;
	}

	errno = ENOTSUP;
	syslog(LOG_WARNING, "Virtual device: %m");

	return -1;
}
