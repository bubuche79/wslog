/*
 * Davis Vantage support.
 */

#ifndef _VANTAGE_H
#define _VANTAGE_H

#include <time.h>
#include <sys/types.h>

#include "driver/driver.h"
#include "dataset.h"

#ifdef __cplusplus
extern "C" {
#endif

int vantage_init(void);
int vantage_destroy(void);

int vantage_get_itimer(struct itimerspec *p, enum ws_timer type);
int vantage_get_loop(struct ws_loop *p);
ssize_t vantage_get_archive(struct ws_archive *p, size_t nel);

int vantage_set_artimer(long itmin, long next);

#ifdef __cplusplus
}
#endif

#endif /* _VANTAGE_H */
