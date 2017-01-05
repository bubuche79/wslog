/*
 * Worker module.
 */

#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>

#include "board.h"
#include "conf.h"
#include "worker.h"

static int startup = 1;
//static pthread_t th[2];

static int
spawn_threads(void)
{
	return 0;
}

static int
worker_destroy(void)
{
	board_unlink();

	return 0;
}

int
worker_main(void)
{
	int errsv;

//	if (set_signals() == -1) {
//		syslog(LOG_ERR, "set_signals: %m");
//		goto error;
//	}

//	post_config();

	/* Startup initialization */
	if (startup) {
		if (board_open(0) == -1) {
			syslog(LOG_EMERG, "board_open(): %m");
			goto error;
		}

		syslog(LOG_INFO, "board_open(): success");
		startup = 0;
	}

	sleep(120);

	/* Spawn all threads */
	if (spawn_threads() == -1) {
		syslog(LOG_EMERG, "spawn_threads(): %m");
		goto error;
	}

	if (worker_destroy() == -1) {
		syslog(LOG_ERR, "worker_destroy(): %m");
	}

	return 0;

error:
	errsv = errno;
	(void) worker_destroy();

	errno = errsv;
	return -1;
}
