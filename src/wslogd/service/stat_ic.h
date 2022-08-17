#ifndef _SERVICE_STAT_IC_H
#define _SERVICE_STAT_IC_H

#include <time.h>

#include "dataset.h"

#ifdef __cplusplus
extern "C" {
#endif

int static_init(int *flags, struct itimerspec *it);
int static_destroy(void);

int static_sig_timer(void);
int static_sig_rt(const struct ws_loop *rt);
int static_sig_ar(const struct ws_archive *ar);

#ifdef __cplusplus
}
#endif

#endif /* _SERVICE_STAT_IC_H */
