#ifndef _BOARD_H
#define _BOARD_H

#include <pthread.h>

/*
 * Shared board.
 */

#ifdef __cplusplus
extern "C" {
#endif

struct ws_board
{
	pthread_rwlock_t rwlock;
};

struct ws_board *boardp;

int board_open(int oflag);
int board_unlink(void);

#ifdef __cplusplus
}
#endif

#endif /* _BOARD_H */
