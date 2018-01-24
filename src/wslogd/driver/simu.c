#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>
#include <errno.h>

#include "driver/driver.h"
#include "driver/simu.h"

#define IODELAY				250
#define PI                  3.1415926535897932384626433832795
#define RAD                 (PI/180.0)
#define PERIOD_FACTOR       4

#define LOOP_INTERVAL		10

static struct timespec simu_delay;
static volatile int simu_index;

static double
simu_sin(double from, double to, int idx)
{
	double range = (to - from) / 2;

	return from + range + sin(idx * RAD / PERIOD_FACTOR) * range;
}

static int
simu_io()
{
	return clock_nanosleep(CLOCK_REALTIME, 0, &simu_delay, NULL);
}

static void
simu_make(struct ws_loop *p, int idx)
{
	p->pressure = simu_sin(950, 1020, idx);
	p->temp = simu_sin(15, 25, idx);
	p->humidity = simu_sin(60, 80, idx);
	p->wind_speed = simu_sin(0, 2, idx);
	p->wind_dir = simu_sin(225, 315, idx);

	/* Supported sensors */
	p->wl_mask = WF_PRESSURE|WF_TEMP|WF_HUMIDITY|WF_WIND_SPEED|WF_WIND_DIR;
}

int
simu_init(void)
{
	simu_index = 0;

	simu_delay.tv_sec = IODELAY / 1000;
	simu_delay.tv_nsec = IODELAY - simu_delay.tv_sec;

	if (simu_io() == -1) {
		return -1;
	}

	return 0;
}

int
simu_destroy(void)
{
	if (simu_io() == -1) {
		return -1;
	}

	return 0;
}

int
simu_get_itimer(struct itimerspec *it, enum ws_timer type)
{
	int ret;

	if (simu_io() == -1) {
		return -1;
	}

	switch (type)
	{
	case WS_ITIMER_LOOP:
		it->it_interval.tv_nsec = 0;
		it->it_interval.tv_sec = LOOP_INTERVAL;
		it->it_value.tv_sec = 0;
		it->it_value.tv_nsec = 0;
		ret = 0;
		break;
	case WS_ITIMER_ARCHIVE:
		errno = ENOTSUP;
		ret = -1;
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
	int idx = simu_index;


	simu_make(p, idx);

	/* Next simulator index */
	simu_index = (idx + 1) % (360 * PERIOD_FACTOR);

	return 0;
}

ssize_t
simu_get_archive(struct ws_archive *p, size_t nel, time_t after)
{
	errno = ENOTSUP;
	return -1;
}
