#ifndef _DRV_DRIVER_H
#define _DRV_DRIVER_H

#include <time.h>
#include <sys/types.h>

#include "dataset.h"

#ifdef __cplusplus
extern "C" {
#endif

enum ws_timer
{
	WS_ITIMER_LOOP,
	WS_ITIMER_ARCHIVE
};

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

int drv_get_itimer(struct itimerspec *p, enum ws_timer);
int drv_get_loop(struct ws_loop *p);

ssize_t drv_get_archive(struct ws_archive *p, size_t nel, time_t after);

int drv_time(time_t *time);
int drv_settime(time_t time);
int drv_set_artimer(long itmin, long next);

#ifdef __cplusplus
}
#endif

#endif /* _DRV_DRIVER_H */
