#ifndef _WS23XX_H
#define _WS23XX_H

#include <time.h>

#include "board.h"

/*
 * Lacrosse Technology WS23XX support.
 */

#ifdef __cplusplus
extern "C" {
#endif

int ws23xx_init(void);
int ws23xx_get_itimer(struct itimerspec *p, int type);
int ws23xx_get_loop(struct ws_loop *p);
int ws23xx_destroy(void);

#ifdef __cplusplus
}
#endif

#endif /* _WS23XX_H */
