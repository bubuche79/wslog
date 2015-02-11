/*
 * Lacrosse Technology WS23XX support.
 */

#ifndef _DRV_WS23XX_H
#define _DRV_WS23XX_H

#include <time.h>
#include <sys/types.h>

#include "dataset.h"

#ifdef __cplusplus
extern "C" {
#endif

int ws23xx_init(void);
int ws23xx_destroy(void);

int ws23xx_get_itimer(struct itimerspec *p, enum ws_timer type);
int ws23xx_get_loop(struct ws_loop *p);
ssize_t ws23xx_get_archive(struct ws_archive *p, size_t nel, time_t after);

int ws23xx_set_artimer(long itmin, long next);

#ifdef __cplusplus
}
#endif

#endif /* _DRV_WS23XX_H */
