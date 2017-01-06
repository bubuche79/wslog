/*
 * Worker module.
 */

#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>

#include "defs/std.h"

#include "board.h"
#include "conf.h"
#include "csv.h"
#include "worker.h"

#define PTHREAD_NONE ((pthread_t) -1)

static int startup = 1;

static pthread_t pth[1];

static int
spawn_threads(void)
{
	size_t i;

	for (i = 0; i < array_size(pth); i++) {
		pth[i] = PTHREAD_NONE;
	}

	if (csv_init() == -1) {
		syslog(LOG_EMERG, "csv_init(): %m");
		return -1;
	}
	if (pthread_create(&pth[0], NULL, csv_run, NULL) == -1) {
		syslog(LOG_EMERG, "pthread_create(csv): %m");
		return -1;
	}

	return 0;
}

static int
worker_destroy(void)
{
	size_t i;
	int ret = 0;

	/* Cancel threads */
	for (i = 0; i < array_size(pth); i++) {
		pthread_t th = pth[i];

		if (th != PTHREAD_NONE) {
			if (pthread_cancel(th) == -1) {
				ret = -1;
			}
			if (pthread_join(th, NULL) == -1) {
				ret = -1;
			}

			pth[i] = PTHREAD_NONE;
		}
	}

	/* Unlink shared board */
	if (board_unlink() == -1) {
		ret = -1;
	}

	return ret;
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

	/* Spawn all threads */
	if (spawn_threads() == -1) {
		syslog(LOG_EMERG, "spawn_threads(): %m");
		goto error;
	}

	sleep(120);

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
