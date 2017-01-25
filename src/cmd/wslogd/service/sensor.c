#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <time.h>
#include <syslog.h>
#ifdef DEBUG
#include <stdio.h>
#endif

#ifdef HAVE_WS23XX
#include "driver/ws23xx.h"
#endif
#ifdef HAVE_SIMU
#include "driver/simu.h"
#endif
#include "board.h"
#include "wslogd.h"
#include "service/service.h"

static enum ws_driver driver;

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
sensor_init(void)
{
	int ret;

	driver = confp->station.driver;

	/* Initialize driver */
	switch (driver)
	{
#ifdef HAVE_WS23XX
	case WS23XX:
		ret = ws23xx_init();
		break;
#endif
#ifdef HAVE_SIMU
	case SIMU:
		ret = simu_init();
		break;
#endif
	default:
		syslog(LOG_ERR, "No supported driver");
		ret = -1;
		break;
	}

	return ret;
}

int
sensor_main(struct timespec *timer)
{
	int ret;
	struct ws_loop loop;

#if DEBUG
	printf("LOOP: reading\n");
#endif

	if (clock_gettime(CLOCK_REALTIME, &loop.time) == -1) {
		goto error;
	}

	/* Read sensors */
	switch (driver)
	{
#ifdef HAVE_WS23XX
	case WS23XX:
		ret = ws23xx_fetch(&loop, timer);
		break;
#endif
#ifdef HAVE_SIMU
	case SIMU:
		ret = simu_fetch(&loop, timer);
		break;
#endif
	default:
		ret = -1;
		break;
	}

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
	printf("LOOP read: %.2f°C %hhu%%\n", loop.temp, loop.humidity);
#endif

	return 0;

error:
	return -1;
}

int
sensor_destroy(void)
{
	int ret;

	/* Destroy driver */
	switch (driver)
	{
#ifdef HAVE_WS23XX
	case WS23XX:
		ret = ws23xx_destroy();
		break;
#endif
#ifdef HAVE_SIMU
	case SIMU:
		ret = simu_destroy();
		break;
#endif
	default:
		ret = -1;
		break;
	}

	return ret;
}
