#ifndef _SIMU_H
#define _SIMU_H

#include <time.h>

#include "board.h"

/*
 * Simulated device.
 *
 * Reads from a file for a few sensors.
 */

#ifdef __cplusplus
extern "C" {
#endif

int simu_init(void);
int simu_fetch(struct ws_loop *p, struct timespec *ts);
int simu_destroy(void);

#ifdef __cplusplus
}
#endif

#endif /* _SIMU_H */
