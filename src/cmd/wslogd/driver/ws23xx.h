#ifndef _WS23XX_H
#define _WS23XX_H

#include <time.h>
#include <sys/types.h>

#include "dataset.h"

/*
 * Lacrosse Technology WS23XX support.
 */

#ifdef __cplusplus
extern "C" {
#endif

int ws23xx_init(void);
int ws23xx_destroy(void);

int ws23xx_get_itimer(struct itimerspec *p, int type);
int ws23xx_get_loop(struct ws_loop *p);
ssize_t ws23xx_get_archive(struct ws_archive *p, size_t nel);

int ws23xx_set_artimer(long itmin, long next);

#ifdef __cplusplus
}
#endif

#endif /* _WS23XX_H */
