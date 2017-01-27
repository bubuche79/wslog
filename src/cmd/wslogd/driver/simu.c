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

static int simu_index;

static double
simu_sin(double from, double to)
{
	double range = (to - from) / 2;

	return from + range + sin(simu_index * RAD / PERIOD_FACTOR) * range;
}

int
simu_init(void)
{
	simu_index = 0;

	return 0;
}

int
simu_get_itimer(struct itimerspec *p, int type)
{
	int ret;

	switch (type)
	{
	case WS_ITIMER_LOOP:
	case WS_ITIMER_ARCHIVE:
		p->it_interval.tv_nsec = 0;
		p->it_interval.tv_sec = (type == WS_ITIMER_LOOP) ? 10 : 300;
		p->it_value.tv_sec = 0;
		p->it_value.tv_nsec = 0;
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
	p->abs_pressure = simu_sin(950, 1020);
	p->temp = simu_sin(15, 25);
	p->humidity = simu_sin(60, 80);
	p->wind_speed = simu_sin(0, 2);
	p->wind_dir = simu_sin(225, 315);

	/* Supported sensors */
	p->wl_mask = WF_PRESSURE|WF_TEMP|WF_HUMIDITY|WF_WIND;

	/* Next simulator index */
	simu_index = (simu_index + 1) % (360 * PERIOD_FACTOR);

	return 0;
}

int
simu_destroy(void)
{
	return 0;
}
