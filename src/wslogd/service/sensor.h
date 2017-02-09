#ifndef _SERVICE_SENSOR_H
#define _SERVICE_SENSOR_H

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

int sensor_init(struct itimerspec *it);
int sensor_main(void);
int sensor_destroy(void);

#ifdef __cplusplus
}
#endif

#endif /* _SERVICE_SENSOR_H */
