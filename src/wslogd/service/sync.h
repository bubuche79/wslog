#ifndef _SERVICE_SYNC_H
#define _SERVICE_SYNC_H

/**
 * Time synchronization support.
 */

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

int sync_init(int *flags, struct itimerspec *it);
int sync_destroy(void);

int sync_sig_timer(void);

#ifdef __cplusplus
}
#endif

#endif	/* _SERVICE_SYNC_H */
