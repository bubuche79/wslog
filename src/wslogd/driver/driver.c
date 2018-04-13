
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <time.h>
#include <errno.h>

#ifdef HAVE_VANTAGE
#include "driver/vantage.h"
#endif
#ifdef HAVE_WS23XX
#include "driver/ws23xx.h"
#endif
#ifdef HAVE_VIRT
#include "driver/virt.h"
#endif
#include "driver/driver.h"
#include "conf.h"

static enum ws_driver driver;

int
drv_init(void)
{
	int ret;

	driver = confp->station.driver;

	switch (driver)
	{
#ifdef HAVE_VANTAGE
	case VANTAGE:
		ret = vantage_init();
		break;
#endif
#ifdef HAVE_WS23XX
	case WS23XX:
		ret = ws23xx_init();
		break;
#endif
#ifdef HAVE_VIRT
	case VIRT:
		ret = virt_init();
		break;
#endif
	default:
		errno = ENOTSUP;
		ret = -1;
		break;
	}

	return ret;
}

int
drv_destroy(void)
{
	int ret;

	switch (driver)
	{
#ifdef HAVE_VANTAGE
	case VANTAGE:
		ret = vantage_destroy();
		break;
#endif
#ifdef HAVE_WS23XX
	case WS23XX:
		ret = ws23xx_destroy();
		break;
#endif
#ifdef HAVE_VIRT
	case VIRT:
		ret = virt_destroy();
		break;
#endif
	default:
		errno = ENOTSUP;
		ret = -1;
		break;
	}

	return ret;
}

int
drv_get_itimer(struct itimerspec *itimer, enum ws_timer type)
{
	int ret;

	switch (driver)
	{
#ifdef HAVE_VANTAGE
	case VANTAGE:
		ret = vantage_get_itimer(itimer, type);
		break;
#endif
#ifdef HAVE_WS23XX
	case WS23XX:
		ret = ws23xx_get_itimer(itimer, type);
		break;
#endif
#ifdef HAVE_VIRT
	case VIRT:
		ret = virt_get_itimer(itimer, type);
		break;
#endif
	default:
		errno = ENOTSUP;
		ret = -1;
		break;
	}

	return ret;
}

int
drv_get_loop(struct ws_loop *loop)
{
	int ret;

	switch (driver)
	{
#ifdef HAVE_VANTAGE
	case VANTAGE:
		ret = vantage_get_loop(loop);
		break;
#endif
#ifdef HAVE_WS23XX
	case WS23XX:
		ret = ws23xx_get_loop(loop);
		break;
#endif
#ifdef HAVE_VIRT
	case VIRT:
		ret = virt_get_loop(loop);
		break;
#endif
	default:
		errno = ENOTSUP;
		ret = -1;
		break;
	}

	return ret;
}

ssize_t
drv_get_archive(struct ws_archive *ar, size_t nel, time_t after)
{
	ssize_t ret;

	switch (driver)
	{
#ifdef HAVE_VANTAGE
	case VANTAGE:
		ret = vantage_get_archive(ar, nel, after);
		break;
#endif
#ifdef HAVE_WS23XX
	case WS23XX:
		ret = ws23xx_get_archive(ar, nel, after);
		break;
#endif
#ifdef HAVE_VIRT
	case VIRT:
		ret = virt_get_archive(ar, nel, after);
		break;
#endif
	default:
		errno = ENOTSUP;
		ret = -1;
		break;
	}

	return ret;
}

/**
 * Set archive interval timer.
 *
 * The interval timer is set to {@code imin} minutes, and the next archive
 * sample is set to {@code next} minutes.
 */
int
drv_set_artimer(long itmin, long next)
{
	ssize_t ret;

	switch (driver)
	{
#ifdef HAVE_WS23XX
	case WS23XX:
		ret = ws23xx_set_artimer(itmin, next);
		break;
#endif
	default:
		errno = ENOTSUP;
		ret = -1;
		break;
	}

	return ret;
}
