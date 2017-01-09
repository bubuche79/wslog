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

struct daemon_thread {
	int dt_signo;									/* signal number */
	struct timespec dt_ifreq;						/* timer interval */
	int (*dt_action) (void);						/* action on signal */

	timer_t dt_timer;
	pthread_t dt_thread;
};

static int startup = 1;

static volatile sig_atomic_t shutdown_pending;
static volatile sig_atomic_t hangup_pending;

static struct daemon_thread csv;
static struct daemon_thread wunder;

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

	/* Create timer */
	se.sigev_notify = SIGEV_SIGNAL;
	se.sigev_signo = dt->dt_signo;
	se.sigev_value.sival_ptr = &dt->dt_timer;

	itimer.it_interval.tv_sec = dt->dt_ifreq.tv_sec;
	itimer.it_interval.tv_nsec = dt->dt_ifreq.tv_nsec;
	itimer.it_value.tv_sec = 1;
	itimer.it_value.tv_nsec = 0;

	if (timer_create(CLOCK_REALTIME, &se, &timer) == -1) {
		syslog(LOG_ERR, "timer_create(): %m");
		goto error;
	}
	if (timer_settime(timer, 0, &itimer, NULL) == -1) {
		syslog(LOG_ERR, "timer_settime(): %m");
		goto error;
	}

	/* Signal notifications */
	while (!shutdown_pending && !hangup_pending) {
		int ret;
		siginfo_t info;

		ret = sigwaitinfo(&set, &info);
		if (ret == -1) {
			syslog(LOG_ERR, "sigwaitinfo(): %m");
			goto error;
		} else {
			if (hangup_pending || shutdown_pending) {
				/* Stop requested */
			} else {
				if (dt->dt_action != NULL) {
					ret = dt->dt_action();
				} else {
					syslog(LOG_ERR, "Real-time signal %d", ret - SIGRTMIN);
					goto error;
				}
			}
		}
	}

	(void) timer_delete(&timer);

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
	int signo;

	/* Signal */
	signo = SIGRTMIN;

	/* CSV thread */
	if (!confp->csv.disabled) {
		csv.dt_signo = signo;
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

		signo++;
	}

	/* Wunder thread */
//	if (!confp->wunder.disabled) {
//		wunder.dt_signo = signo;
//		wunder.dt_ifreq.tv_sec = confp->wunder.freq;
//		wunder.dt_ifreq.tv_nsec = 0;
////		csv.dt_action = wunder_write;
//
//		if (pthread_create(&wunder.dt_thread, NULL, pthread_main, &wunder) == -1) {
//			syslog(LOG_EMERG, "pthread_create(wunder): %m");
//			return -1;
//		}
//
//		signo++;
//	}
//
	return 0;
}

static int
worker_destroy(void)
{
	size_t i;
	int ret = 0;
	void *th_res;

	pthread_kill(csv.dt_thread, csv.dt_signo);
	pthread_join(csv.dt_thread, &th_res);

	syslog(LOG_INFO, "threads stopped");

	/* Shutdown requested */
	if (shutdown_pending) {
		/* Unlink shared board */
		if (board_unlink() == -1) {
			ret = -1;
		}

		syslog(LOG_INFO, "resources released");
	}

	return ret;
}

int
worker_main(int *halt)
{
	int errsv;

	shutdown_pending = 0;
	hangup_pending = 0;

	/* Startup initialization */
	if (startup) {
		if (sig_block() == -1) {
			syslog(LOG_ERR, "set_signals: %m");
			return -1;
		}

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
