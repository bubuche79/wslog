#ifndef _SERVICE_SERVICE_H
#define _SERVICE_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

int sensor_init(void);
int sensor_main(struct timespec *);
int sensor_destroy(void);

#ifdef __cplusplus
}
#endif

#endif /* _SERVICE_SERVICE_H */
