#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <time.h>
#include <string.h>
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

struct drv {
	enum ws_driver driver;

	int (*destroy)();

	int (*get_rt)(struct ws_loop *);
	int (*get_rt_itimer)(struct itimerspec *);

	ssize_t (*get_ar)(struct ws_archive *, size_t, time_t);
	int (*get_ar_itimer)(struct itimerspec *);

	int (*get_time)(time_t *);
	int (*set_time)(time_t);
};

static struct drv drv;

int
drv_init(void)
{
	int ret;

	memset(&drv, 0, sizeof(drv));

	drv.driver = confp->station.driver;

	switch (drv.driver)
	{
#ifdef HAVE_VANTAGE
	case VANTAGE:
		drv.destroy = vantage_destroy;
		drv.get_rt = vantage_get_rt;
		drv.get_rt_itimer = vantage_get_rt_itimer;
		drv.get_ar = vantage_get_ar;
		drv.get_ar_itimer = vantage_get_ar_itimer;
		drv.get_time = vantage_time;
		drv.set_time = vantage_adjtime;

		ret = vantage_init();
		break;
#endif
#ifdef HAVE_WS23XX
	case WS23XX:
		drv.destroy = ws23xx_destroy;
		drv.get_rt = ws23xx_get_rt;
		drv.get_rt_itimer = ws23xx_get_rt_itimer;
		drv.get_ar = ws23xx_get_ar;
		drv.get_ar_itimer = ws23xx_get_ar_itimer;

		ret = ws23xx_init();
		break;
#endif
#ifdef HAVE_VIRT
	case VIRT:
		drv.destroy = virt_destroy;
		drv.get_rt = virt_get_rt;
		drv.get_rt_itimer = virt_get_rt_itimer;
		drv.get_ar = virt_get_ar;
		drv.get_ar_itimer = virt_get_ar_itimer;
		drv.get_time = virt_time;
		drv.set_time = virt_adjtime;

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

	if (drv.destroy != NULL) {
		ret = drv.destroy();
	} else {
		ret = -1;
		errno = ENOTSUP;
	}

	return ret;
}

int
drv_get_rt(struct ws_loop *loop)
{
	int ret;

	if (drv.get_rt != NULL) {
		ret = drv.get_rt(loop);
	} else {
		ret = -1;
		errno = ENOTSUP;
	}

	return ret;
}

int
drv_get_rt_itimer(struct itimerspec *itimer)
{
	int ret;

	if (drv.get_rt_itimer != NULL) {
		ret = drv.get_rt_itimer(itimer);
	} else {
		ret = -1;
		errno = ENOTSUP;
	}

	return ret;
}

ssize_t
drv_get_ar(struct ws_archive *ar, size_t nel, time_t after)
{
	int ret;

	if (drv.get_ar != NULL) {
		ret = drv.get_ar(ar, nel, after);
	} else {
		ret = -1;
		errno = ENOTSUP;
	}

	return ret;
}

int
drv_get_ar_itimer(struct itimerspec *itimer)
{
	int ret;

	if (drv.get_ar_itimer != NULL) {
		ret = drv.get_ar_itimer(itimer);
	} else {
		ret = -1;
		errno = ENOTSUP;
	}

	return ret;
}

int
drv_time(time_t *time)
{
	int ret;

	if (drv.get_time != NULL) {
		ret = drv.get_time(time);
	} else {
		ret = -1;
		errno = ENOTSUP;
	}

	return ret;
}

int
drv_settime(time_t time)
{
	int ret;

	if (drv.set_time != NULL) {
		ret = drv.set_time(time);
	} else {
		ret = -1;
		errno = ENOTSUP;
	}

	return ret;
}
