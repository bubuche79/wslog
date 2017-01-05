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

int
worker_main(void)
{
	int errsv;

//	if (set_signals() == -1) {
//		syslog(LOG_ERR, "set_signals: %m");
//		goto error;
//	}

	post_config();

	/* Startup initialization */
	if (startup) {
		if (board_open(O_RDWR) == 1) {
			syslog(LOG_EMERG, "init_shmem: %m");
			goto error;
		}

		startup = 0;
	}

	/* Create initial pool of child servers */
	if (startup_children(start_servers, 0) == -1) {
		syslog(LOG_EMERG, "startup_children(%d): %m", start_servers);
		goto error;
	}

	/*
	 * From now on, manage the pool of spare servers, and launch new
	 * ones or kill some idle ones to keep their number as requested.
	 */
	while (!shutdown_pending && !hangup_pending) {
		sigset_t sigmask;
		siginfo_t info;
		struct timespec timeout;

		/* Signals to catch */
		(void) sigemptyset(&sigmask);
		(void) sigaddset(&sigmask, SIGHUP);
		(void) sigaddset(&sigmask, SIGTERM);
		(void) sigaddset(&sigmask, SIGCHLD);

		timeout.tv_sec = SIGWAIT_TIMEOUT / 1000;
		timeout.tv_nsec = (SIGWAIT_TIMEOUT % 1000) * 1000000;

		if (sigtimedwait(&sigmask, &info, &timeout) > 0) {
			switch (info.si_signo) {
			case SIGHUP:
				hangup_pending = 1;
				break;
			case SIGTERM:
				shutdown_pending = 1;
				break;
			case SIGCHLD:
				if (reap_children() == -1) {
					syslog(LOG_EMERG, "reap_children: %m");
					goto error;
				}
				break;
			}
		}
	}

	/* Release resources */
	*halt = shutdown_pending;

	return prefork_destroy(shutdown_pending);

error:
	errsv = errno;
	(void) prefork_destroy(1);

	errno = errsv;
	return -1;
}
