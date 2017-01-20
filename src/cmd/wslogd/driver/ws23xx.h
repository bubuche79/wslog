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
int ws23xx_fetch(struct ws_loop *p, struct timespec *ts);
int ws23xx_destroy(void);

#ifdef __cplusplus
}
#endif

#endif /* _WS23XX_H */
