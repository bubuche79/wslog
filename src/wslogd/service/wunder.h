#ifndef _SERVICE_WUNDER_H
#define _SERVICE_WUNDER_H

/**
 * Weather underground support.
 */

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

int wunder_init(struct itimerspec *it);
int wunder_timer(void);
int wunder_destroy(void);

#ifdef __cplusplus
}
#endif

#endif	/* _SERVICE_WUNDER_H */
