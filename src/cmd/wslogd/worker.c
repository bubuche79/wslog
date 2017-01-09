/*
 * Worker module.
 */

#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <time.h>

#include "defs/std.h"

#include "board.h"
#include "conf.h"
#include "csv.h"
#include "wslogd.h"
#include "worker.h"

#define PTHREAD_NONE ((pthread_t) -1)

static int startup = 1;

struct daemon_thread {
	int dt_signo;									/* signal number */
	int (*dt_action) (void);						/* action on signal */
	struct timespec dt_ifreq;						/* timer interval */

	timer_t dt_timer;
	pthread_t dt_thread;
};

static struct daemon_thread csv;

static volatile sig_atomic_t shutdown_pending = 0;
static volatile sig_atomic_t hangup_pending = 0;

static int
sig_block(void)
{
	sigset_t set;

	(void) sigemptyset(&set);
	(void) sigaddset(&set, SIGHUP);
	(void) sigaddset(&set, SIGALRM);
	(void) sigaddset(&set, SIGTERM);

	/* Signals handled by threads */
	(void) sigaddset(&set, SIGRTMIN);

	return pthread_sigmask(SIG_BLOCK, &set, NULL);
}

static void *
pthread_main(void *arg)
{
	int errsv;
	struct daemon_thread *dt;

	sigset_t set;

	timer_t timer;
	struct sigevent se;
	struct itimerspec itimer;

	dt = (struct daemon_thread *) arg;

	/* Change blocked signals */
	(void) sigemptyset(&set);
	(void) sigaddset(&set, dt->dt_signo);
	(void) pthread_sigmask(SIG_BLOCK, &set, NULL);

	printf("(%lu) started\n", pthread_self());

	/* Create timer */
	se.sigev_notify = SIGEV_SIGNAL;
	se.sigev_signo = dt->dt_signo;
	se.sigev_value.sival_ptr = &dt->dt_timer;

	itimer.it_interval.tv_sec = dt->dt_ifreq.tv_sec;
	itimer.it_interval.tv_nsec = dt->dt_ifreq.tv_nsec;
	itimer.it_value.tv_sec = 1;
	itimer.it_value.tv_nsec = 0;

	if (timer_create(CLOCK_REALTIME, &se, &timer) == -1) {
		syslog(LOG_EMERG, "timer_create(): %m");
		goto error;
	}
	if (timer_settime(timer, 0, &itimer, NULL) == -1) {
		syslog(LOG_EMERG, "timer_settime(): %m");
		goto error;
	}

	/* Signal notifications */
	for (;;) {
		int ret;
		siginfo_t info;

		ret = sigwaitinfo(&set, &info);
		if (ret == -1) {
			syslog(LOG_ERR, "(%lu) sigwaitinfo(): %m", pthread_self());
			goto error;
		} else if (dt->dt_action != NULL) {
			ret = dt->dt_action();
		} else {
			syslog(LOG_ERR, "(%lu) %d signal", pthread_self(), ret);
			goto error;
		}
	}

	return NULL;

error:
	errsv = errno;
	(void) timer_delete(&timer);

	errno = errsv;
	return NULL;
}

static int
spawn_threads(void)
{
	/* CSV thread */
	if (!confp->csv.disabled) {
		csv.dt_signo = SIGRTMIN;
		csv.dt_ifreq.tv_sec = confp->csv.freq;
		csv.dt_ifreq.tv_nsec = 0;
		csv.dt_action = csv_write;

		if (csv_init() == -1) {
			syslog(LOG_EMERG, "csv_init(): %m");
			return -1;
		}
		if (pthread_create(&csv.dt_thread, NULL, pthread_main, &csv) == -1) {
			syslog(LOG_EMERG, "pthread_create(csv): %m");
			return -1;
		}
	}

	return 0;
}

static int
worker_destroy(void)
{
	size_t i;
	int ret = 0;

#if 0
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
#endif

//	if (timer_stop() == -1) {
//		syslog(LOG_ERR, "timer_stop(): %m");
//		ret = -1;
//	}

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

	shutdown_pending = 0;
	hangup_pending = 0;

	if (sig_block() == -1) {
		syslog(LOG_ERR, "set_signals: %m");
		return -1;
	}

	/* Startup initialization */
	if (startup) {
		if (board_open(0) == -1) {
			syslog(LOG_EMERG, "board_open(): %m");
			goto error;
		}

		syslog(LOG_INFO, "board_open(): success");
		startup = 0;
	}


	printf("(%lu) main\n", pthread_self());

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
				syslog(LOG_ERR, "Signal: %d", ret);
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
