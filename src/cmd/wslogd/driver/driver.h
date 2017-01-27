#ifndef _DRIVER_H
#define _DRIVER_H

#include <time.h>

#include "board.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
	WS_ITIMER_LOOP,
	WS_ITIMER_ARCHIVE
};

enum ws_driver {
#if HAVE_WS23XX
	WS23XX,								/* La Crosse Technology WS23XX */
#endif
#if HAVE_SIMU
	SIMU,								/* simulator device */
#endif
	UNUSED
};

int drv_init(void);
int drv_get_itimer(struct itimerspec *p, int type);
int drv_get_loop(struct ws_loop *p);
int drv_destroy(void);

#ifdef __cplusplus
}
#endif

#endif /* _DRIVER_H */
