#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>
#include <errno.h>

#include "driver/driver.h"
#include "driver/simu.h"

#define PI                  3.1415926535897932384626433832795
#define RAD                 (PI/180.0)
#define PERIOD_FACTOR       4

#define ARCHIVE_INTERVAL	300
#define LOOP_INTERVAL		10

static volatile int simu_index;

static double
simu_sin(double from, double to, int idx)
{
	double range = (to - from) / 2;

	return from + range + sin(idx * RAD / PERIOD_FACTOR) * range;
}

static void
simu_make(struct ws_loop* p, int idx) {
	p->abs_pressure = simu_sin(950, 1020, idx);
	p->temp = simu_sin(15, 25, idx);
	p->humidity = simu_sin(60, 80, idx);
	p->wind_speed = simu_sin(0, 2, idx);
	p->wind_dir = simu_sin(225, 315, idx);

	/* Supported sensors */
	p->wl_mask = WF_PRESSURE | WF_TEMP | WF_HUMIDITY | WF_WIND;
}

int
simu_init(void)
{
	simu_index = 0;

	return 0;
}

int
simu_get_itimer(struct itimerspec *it, int type)
{
	int ret;

	switch (type)
	{
	case WS_ITIMER_LOOP:
	case WS_ITIMER_ARCHIVE:
		it->it_interval.tv_nsec = 0;
		if (type == WS_ITIMER_LOOP) {
			it->it_interval.tv_sec = LOOP_INTERVAL;
		} else {
			it->it_interval.tv_sec = ARCHIVE_INTERVAL;
		}
		it->it_value.tv_sec = 0;
		it->it_value.tv_nsec = 0;
		ret = 0;
		break;
	default:
		errno = ENOTSUP;
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
simu_get_archive(struct ws_archive *p, size_t nel)
{
	int idx = simu_index;

	if (nel > 1) {
		return -1;
	}

	p->time = time(NULL);
	p->interval = ARCHIVE_INTERVAL;

	p->data.time.tv_sec = p->time;
	p->data.time.tv_nsec = 0;
	simu_make(&p->data, idx);

	return nel;
}

int
simu_destroy(void)
{
	return 0;
}
