#ifndef _BOARD_H
#define _BOARD_H

#include "dataset.h"

/*
 * Shared board.
 */

#ifdef __cplusplus
extern "C" {
#endif

int board_open(int oflag);
int board_unlink(void);

int board_lock(void);
int board_unlock(void);

void board_push(const struct ws_loop *p);
void board_push_ar(const struct ws_archive *p);

struct ws_loop *board_peek(size_t i);
struct ws_archive *board_peek_ar(size_t i);

#ifdef __cplusplus
}
#endif

#endif /* _BOARD_H */
