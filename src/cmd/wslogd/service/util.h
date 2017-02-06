#ifndef _SERVICE_UTIL_H
#define _SERVICE_UTIL_H

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

void itimer_set(struct itimerspec* it, long sec);
void itimer_add_delay(struct itimerspec *it, const struct itimerspec *ref, long delay);

#ifdef __cplusplus
}
#endif

#endif /* _SERVICE_UTIL_H */
