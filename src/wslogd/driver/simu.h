/*
 * Simulated device.
 *
 * Reads random data for a few sensors.
 */

#ifndef _SIMU_H
#define _SIMU_H

#include <time.h>
#include <sys/types.h>

#include "driver/driver.h"
#include "dataset.h"

#ifdef __cplusplus
extern "C" {
#endif

int simu_init(void);
int simu_destroy(void);

int simu_get_itimer(struct itimerspec *it, enum ws_type type);
int simu_get_loop(struct ws_loop *p);
ssize_t simu_get_archive(struct ws_archive *p, size_t nel);

#ifdef __cplusplus
}
#endif

#endif /* _SIMU_H */
