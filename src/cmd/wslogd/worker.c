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
#include "wslogd.h"
#include "worker.h"

#define PTHREAD_NONE ((pthread_t) -1)

static int startup = 1;

static pthread_t csv_th;
static pthread_t wunder_th;

static volatile sig_atomic_t shutdown_pending = 0;
static volatile sig_atomic_t hangup_pending = 0;

static int
unset_signals(void)
{
	sigset_t sigmask;

	/* Unblock signals */
	(void) sigemptyset(&sigmask);
	(void) sigaddset(&sigmask, SIGHUP);
	(void) sigaddset(&sigmask, SIGTERM);
	(void) sigaddset(&sigmask, SIGCHLD);

	return pthread_sigmask(SIG_UNBLOCK, &sigmask, NULL);
}

static int
set_signals(void)
{
	sigset_t sigmask;

	(void) sigemptyset(&sigmask);
	(void) sigaddset(&sigmask, SIGHUP);
	(void) sigaddset(&sigmask, SIGTERM);

	return pthread_sigmask(SIG_BLOCK, &sigmask, NULL);
}

static int
spawn_threads(void)
{
	/* Clear state */
	csv_th = PTHREAD_NONE;
	wunder_th = PTHREAD_NONE;

	/* CSV thread */
	if (!confp->csv.disabled) {
		if (csv_init() == -1) {
			syslog(LOG_EMERG, "csv_init(): %m");
			return -1;
		}
		if (pthread_create(&csv_th, NULL, csv_run, NULL) == -1) {
			syslog(LOG_EMERG, "pthread_create(csv): %m");
			return -1;
		}
	}

	/* Wunderground thread */
	if (!confp->wunder.disabled) {

	}

	return 0;
}

static int
worker_destroy(void)
{
	size_t i;
	int ret = 0;

	pthread_t *pth[] = {
			&csv_th
	};

	/* Cancel threads */
	for (i = 0; i < array_size(pth); i++) {
		pthread_t th = *pth[i];

		if (th != PTHREAD_NONE) {
			if (pthread_cancel(th) == -1) {
				ret = -1;
			}
			if (pthread_join(th, NULL) == -1) {
				ret = -1;
			}

			*pth[i] = PTHREAD_NONE;
		}
	}

	/* Unlink shared board */
	if (board_unlink() == -1) {
		ret = -1;
	}

	return ret;
}

int
worker_main(int *halt)
{
	int errsv;

	if (set_signals() == -1) {
		syslog(LOG_ERR, "set_signals: %m");
		return -1;
	}

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

	/* Wait for events */
	while (!shutdown_pending && !hangup_pending) {
		int ret;
		sigset_t set;
		siginfo_t info;

		/* Signals to wait for */
		(void) sigemptyset(&set);
		(void) sigaddset(&set, SIGHUP);
		(void) sigaddset(&set, SIGTERM);

		ret = sigwaitinfo(&set, &info);
		if (ret == -1) {
			syslog(LOG_ERR, "sigwaitinfo(): %m");
		} else {
			switch (ret) {
			case SIGHUP:
				hangup_pending = 1;
				syslog(LOG_NOTICE, "HUP signal received");
				break;
			case SIGTERM:
				shutdown_pending = 1;
				syslog(LOG_NOTICE, "TERM signal received");
				break;
			default:
				break;
			}
		}
	}

	/* Release resources */
	*halt = shutdown_pending;

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
