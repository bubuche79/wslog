#ifndef _WUNDER_H
#define _WUNDER_H

#include <time.h>

/**
 * Weather underground support.
 */

#ifdef __cplusplus
extern "C" {
#endif

int wunder_init(void);
int wunder_update(struct timespec *timer);
int wunder_destroy(void);

#ifdef __cplusplus
}
#endif

#endif	/* _WUNDER_H */
