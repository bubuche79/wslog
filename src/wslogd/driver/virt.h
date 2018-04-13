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

int virt_get_itimer(struct itimerspec *it, enum ws_timer type);
int virt_get_loop(struct ws_loop *p);
ssize_t virt_get_archive(struct ws_archive *p, size_t nel, time_t after);

#ifdef __cplusplus
}
#endif

#endif /* _DRV_VIRT_H */
