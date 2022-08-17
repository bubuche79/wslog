#ifndef _SERVICE_SENSOR_H
#define _SERVICE_SENSOR_H

#include <time.h>

#include "dataset.h"

#ifdef __cplusplus
extern "C" {
#endif

int sensor_init(struct itimerspec *it);
int sensor_destroy(void);

int sensor_sig_timer(struct ws_loop *p);

#ifdef __cplusplus
}
#endif

#endif /* _SERVICE_SENSOR_H */
