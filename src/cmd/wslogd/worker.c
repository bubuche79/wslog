/*
 * Worker module.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <time.h>
#ifdef HAVE_SIGTHREADID
#include <linux/version.h>
#include <unistd.h>
#include <sys/syscall.h>
#endif

#include "defs/std.h"
#include "libws/log.h"

#include "board.h"
#include "conf.h"
#include "sqlite.h"
#include "wunder.h"
#include "wslogd.h"
#include "service/service.h"
#include "worker.h"

struct worker {
	int w_signo;									/* signal number */
	struct timespec w_ifreq;						/* timer interval */
	int (*w_init) (void);
	int (*w_action) (void);							/* action on signal */
	int (*w_destroy) (void);

	timer_t w_timer;
	pthread_t w_thread;								/* thread id */
};

static int startup = 1;

static volatile sig_atomic_t shutdown_pending;
static volatile sig_atomic_t hangup_pending;

static struct worker threads[4];					/* daemon threads */
static size_t threads_nel;							/* number of elements */

static void
sig_set(sigset_t *set)
{
	(void) sigemptyset(set);
	(void) sigaddset(set, SIGHUP);
	(void) sigaddset(set, SIGTERM);
#ifdef HAVE_SIGTHREADID
	(void) sigaddset(set, SIGALRM);
#endif
}

static void *
sigthread_main(void *arg)
{
	int errsv;
	struct worker *dt;

	sigset_t set;

	timer_t timer;
	struct sigevent se;
	struct itimerspec itimer;

	dt = (struct worker *) arg;

	/* Initialize */
	if (dt->w_init != NULL) {
		if (dt->w_init() == -1) {
			return NULL;
		}
	}

	/* Change blocked signals */
	(void) sigemptyset(&set);
	(void) sigaddset(&set, dt->w_signo);

	/* Create timer */
#ifndef HAVE_SIGTHREADID
	se.sigev_notify = SIGEV_SIGNAL;
#else
	se.sigev_notify = SIGEV_THREAD_ID;
	se._sigev_un._tid = syscall(SYS_gettid);
#endif
	se.sigev_signo = dt->w_signo;
	se.sigev_value.sival_ptr = &dt->w_timer;

	itimer.it_interval.tv_sec = dt->w_ifreq.tv_sec;
	itimer.it_interval.tv_nsec = dt->w_ifreq.tv_nsec;
	itimer.it_value.tv_sec = 0;
	itimer.it_value.tv_nsec = 100;

	if (timer_create(CLOCK_REALTIME, &se, &timer) == -1) {
		csyslog1(LOG_ERR, "timer_create(): %m");
		goto error;
	}
	if (timer_settime(timer, 0, &itimer, NULL) == -1) {
		csyslog1(LOG_ERR, "timer_settime(): %m");
		goto error;
	}

	/* Signal notifications */
	while (!shutdown_pending && !hangup_pending) {
		int ret;
		siginfo_t info;

		ret = sigwaitinfo(&set, &info);
		if (ret == -1) {
			csyslog1(LOG_ERR, "sigwaitinfo(): %m");
			goto error;
		} else {
			if (hangup_pending || shutdown_pending) {
				/* Stop requested */
			} else {
				if (dt->w_action != NULL) {
					ret = dt->w_action();
				} else {
					syslog(LOG_ERR, "Real-time signal %d", ret - SIGRTMIN);
					goto error;
				}
			}
		}
	}

	(void) timer_delete(&timer);

	/* Cleanup */
	if (dt->w_destroy != NULL) {
		if (dt->w_destroy() == -1) {
			return NULL;
		}
	}

	return NULL;

error:
	errsv = errno;
	(void) timer_delete(&timer);
	(void) dt->w_destroy();

	errno = errsv;
	return NULL;
}

static int
sigthread_create(struct worker *dt)
{
	if (pthread_create(&dt->w_thread, NULL, sigthread_main, dt) == -1) {
		csyslog1(LOG_EMERG, "pthread_create(): %m");
		return -1;
	}

	return 0;
}

static int
sigthread_kill(struct worker *dt)
{
	int ret;
#ifndef HAVE_SIGTHREADID
	sigset_t set;
#endif

	ret = 0;

	/* Kill thread */
	if (pthread_kill(dt->w_thread, dt->w_signo) == -1) {
		ret = -1;
	} else {
		void *th_res;

		if (pthread_join(dt->w_thread, &th_res) == -1) {
			ret = -1;
		}
	}

#ifndef HAVE_SIGTHREADID
	/* Signal management */
	(void) sigemptyset(&set);
	(void) sigaddset(&set, dt->w_signo);

	if (pthread_sigmask(SIG_UNBLOCK, &set, NULL) == -1) {
		return -1;
	}
#endif

	return ret;
}

static void
threads_count(void)
{
	threads_nel = 1;

	if (!confp->sqlite.disabled) {
		threads_nel++;
	}
	if (!confp->wunder.disabled) {
		threads_nel++;
	}
}

static int
threads_start(void)
{
	int signo;
	size_t i = 0;
#ifndef HAVE_SIGTHREADID
	sigset_t set;
#endif

	threads_count();

	/* Signal */
#ifndef HAVE_SIGTHREADID
	signo = SIGRTMIN;
#else
	signo = SIGALRM;
#endif

	/* Configure sensor thread */
	threads[i].w_signo = signo;
	threads[i].w_ifreq.tv_sec = confp->ws23xx.freq;
	threads[i].w_ifreq.tv_nsec = 0;
	threads[i].w_init = sensor_init;
	threads[i].w_action = sensor_main;
	threads[i].w_destroy = sensor_destroy;

	i++;
#ifndef HAVE_SIGTHREADID
	signo++;
#endif

	/* Configure archive thread */
	if (!confp->sqlite.disabled) {
		threads[i].w_signo = signo;
		threads[i].w_ifreq.tv_sec = confp->sqlite.freq;
		threads[i].w_ifreq.tv_nsec = 0;
		threads[i].w_init = archive_init;
		threads[i].w_action = archive_main;
		threads[i].w_destroy = archive_destroy;

		i++;
#ifndef HAVE_SIGTHREADID
		signo++;
#endif
	}

	/* Configure Wunder thread */
	if (!confp->wunder.disabled) {
		threads[i].w_signo = signo;
		threads[i].w_ifreq.tv_sec = confp->wunder.freq;
		threads[i].w_ifreq.tv_nsec = 0;
		threads[i].w_init = wunder_init;
		threads[i].w_action = wunder_update;
		threads[i].w_destroy = wunder_destroy;

		i++;
#ifndef HAVE_SIGTHREADID
		signo++;
#endif
	}

#ifndef HAVE_SIGTHREADID
	/* Manage signals */
	(void) sigemptyset(&set);

	for (i = 0; i < threads_nel; i++) {
		(void) sigaddset(&set, threads[i].w_signo);
	}

	if (pthread_sigmask(SIG_BLOCK, &set, NULL) == -1) {
		return -1;
	}
#endif

	/* Create threads */
	for (i = 0; i < threads_nel; i++) {
		struct worker *dt = &threads[i];

		if (sigthread_create(dt) == -1) {
			csyslog1(LOG_EMERG, "pthread_create(): %m");
			return -1;
		}
	}

	return 0;
}

static int
threads_kill(void)
{
	int ret;
	size_t i;

	ret = 0;

	for (i = 0; i < threads_nel; i++) {
		struct worker *dt = &threads[i];

		if (sigthread_kill(dt) == -1) {
			ret = -1;
		}
	}

	return ret;
}

static int
worker_destroy(void)
{
	int ret = 0;

	threads_kill();
	syslog(LOG_INFO, "threads stopped");

	/* Shutdown requested */
	if (shutdown_pending) {
		/* Unlink shared board */
		if (board_unlink() == -1) {
			ret = -1;
		}

		syslog(LOG_INFO, "resources released");
	}

	if (drv_destroy() == -1) {
		ret = -1;
	}

	return ret;
}

int
worker_main(int *halt)
{
	int errsv;
	sigset_t set;

	shutdown_pending = 0;
	hangup_pending = 0;

	sig_set(&set);

	/* Open device */
	if (drv_init() == -1) {
		return -1;
	}

	/* Startup initialization */
	if (startup) {
		if (pthread_sigmask(SIG_BLOCK, &set, NULL) == -1) {
			csyslog1(LOG_ERR, "pthread_sigmask: %m");
			return -1;
		}

		if (board_open(O_CREAT) == -1) {
			csyslog1(LOG_EMERG, "board_open(): %m");
			goto error;
		}

		syslog(LOG_INFO, "board_open(): success");
		startup = 0;
	}

	/* Start all workers */
	if (threads_start() == -1) {
		csyslog1(LOG_EMERG, "threads_start(): %m");
		goto error;
	}

	/* Wait for events */
	while (!shutdown_pending && !hangup_pending) {
		int ret;
		siginfo_t info;

		/* Signals to wait for */
		ret = sigwaitinfo(&set, &info);
		if (ret == -1) {
			csyslog1(LOG_ERR, "sigwaitinfo(): %m");
		} else {
			switch (ret) {
			case SIGHUP:
				hangup_pending = 1;
				syslog(LOG_NOTICE, "Signal HUP");
				break;
			case SIGTERM:
				shutdown_pending = 1;
				syslog(LOG_NOTICE, "Signal TERM");
				break;
			default:
				syslog(LOG_ERR, "Signal %d", ret);
				break;
			}
		}
	}

	/* Release resources */
	*halt = shutdown_pending;

	if (worker_destroy() == -1) {
		csyslog1(LOG_ERR, "worker_destroy(): %m");
	}

	return 0;

error:
	errsv = errno;
	(void) worker_destroy();

	errno = errsv;
	return -1;
}
