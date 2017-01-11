#ifndef _DAEMON_H
#define _DAEMON_H

#include <sys/types.h>

/*
 * Daemon related functions.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Run in the background.
 *
 * See daemon(7) for more details.
 */
int daemon_create(void);

#ifdef __cplusplus
}
#endif

#endif /* _DAEMON_H */
