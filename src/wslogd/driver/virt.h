/*
 * Virtual device.
 *
 * Reads random data for a few sensors.
 */

#ifndef _DRV_VIRT_H
#define _DRV_VIRT_H

#include <time.h>
#include <sys/types.h>

#include "driver/driver.h"
#include "dataset.h"

#ifdef __cplusplus
extern "C" {
#endif

int virt_init(void);
int virt_destroy(void);

int virt_get_rt(struct ws_loop *p);
int virt_get_rt_itimer(struct itimerspec *it);

ssize_t virt_get_ar(struct ws_archive *p, size_t nel, time_t after);
int virt_get_ar_itimer(struct itimerspec *it);

int virt_time(time_t *time);
int virt_adjtime(time_t time);

#ifdef __cplusplus
}
#endif

#endif /* _DRV_VIRT_H */
