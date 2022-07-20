#ifndef _DRV_DRIVER_H
#define _DRV_DRIVER_H

#include <time.h>
#include <sys/types.h>

#include "dataset.h"

#ifdef __cplusplus
extern "C" {
#endif

enum ws_driver
{
#if HAVE_VANTAGE
	VANTAGE,			/* Davis Vantage */
#endif
#if HAVE_WS23XX
	WS23XX,				/* La Crosse Technology WS23XX */
#endif
#if HAVE_VIRT
	VIRT,				/* Virtual device */
#endif
	UNUSED
};

int drv_init(void);
int drv_destroy(void);

int drv_get_rt(struct ws_loop *p);
int drv_get_rt_itimer(struct itimerspec *p);

ssize_t drv_get_ar(struct ws_archive *p, size_t nel, time_t after);
int drv_get_ar_itimer(struct itimerspec *p);

int drv_time(time_t *time);
int drv_settime(time_t time);

#ifdef __cplusplus
}
#endif

#endif /* _DRV_DRIVER_H */
