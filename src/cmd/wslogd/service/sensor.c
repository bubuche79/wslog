#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef DEBUG
#include <stdio.h>
#endif
#include <syslog.h>

#ifdef HAVE_WS23XX
#include "driver/ws23xx.h"
#endif
#include "wslogd.h"
#include "service/service.h"

static enum ws_driver driver;

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

	/* Read sensors */
	switch (driver)
	{
#ifdef HAVE_WS23XX
	case WS23XX:
		ret = ws23xx_fetch(&loop, timer);
		break;
#endif
	default:
		ret = -1;
		break;
	}

	if (ret == -1) {
		goto error;
	}

	/* Push loop event in board */
	board_push(&loop);

#if DEBUG
	printf("LOOP read: %.2f %hhu%%\n", loop.temp, loop.humidity);
#endif

error:
	return ret;
}

int
sensor_destroy()
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
	default:
		ret = -1;
		break;
	}

	return ret;
}
