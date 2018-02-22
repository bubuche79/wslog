#ifndef _WUNDER_H
#define _WUNDER_H

/**
 * Weather underground support.
 */

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

int wunder_init(struct itimerspec *it);
int wunder_update(void);
int wunder_destroy(void);

#ifdef __cplusplus
}
#endif

#endif	/* _WUNDER_H */
