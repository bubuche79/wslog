#ifndef _SERVICE_SERVICE_H
#define _SERVICE_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

int sensor_init(void);
int sensor_main(void);
int sensor_destroy(void);

int archive_init(void);
int archive_main(void);
int archive_destroy(void);

#ifdef __cplusplus
}
#endif

#endif /* _SERVICE_SERVICE_H */
