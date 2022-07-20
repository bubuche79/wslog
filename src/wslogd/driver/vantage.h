/*
 * Davis Vantage support.
 */

#ifndef _DRV_VANTAGE_H
#define _DRV_VANTAGE_H

#include <time.h>
#include <sys/types.h>

#include "driver/driver.h"
#include "dataset.h"

#ifdef __cplusplus
extern "C" {
#endif

int vantage_init(void);
int vantage_destroy(void);

int vantage_get_rt(struct ws_loop *p);
int vantage_get_rt_itimer(struct itimerspec *p);

ssize_t vantage_get_ar(struct ws_archive *p, size_t nel, time_t after);
int vantage_get_ar_itimer(struct itimerspec *p);

int vantage_time(time_t *time);
int vantage_adjtime(time_t time);
int vantage_set_artimer(long itmin, long next);

#ifdef __cplusplus
}
#endif

#endif /* _DRV_VANTAGE_H */
