#ifndef _SERVICE_IC_H
#define _SERVICE_IC_H

#include <time.h>

#include "dataset.h"

#ifdef __cplusplus
extern "C" {
#endif

int ic_init(int *flags, struct itimerspec *it);
int ic_destroy(void);

int ic_sig_rt(const struct ws_loop *rt);
int ic_sig_ar(const struct ws_archive *ar);

#ifdef __cplusplus
}
#endif

#endif /* _SERVICE_IC_H */
