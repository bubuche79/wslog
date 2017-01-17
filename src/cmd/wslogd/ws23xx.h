#ifndef _WS23XX_H
#define _WS23XX_H

#include "board.h"

/*
 * Lacrosse Technology WS23XX support.
 */

#ifdef __cplusplus
extern "C" {
#endif

int ws23xx_init(void);
int ws23xx_fetch(struct ws_loop *p);
int ws23xx_destroy(void);

#ifdef __cplusplus
}
#endif

#endif /* _WS23XX_H */
