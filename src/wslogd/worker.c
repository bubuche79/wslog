/*
 * Worker module.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <pthread.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <time.h>

#include "libws/defs.h"

#include "board.h"
#include "conf.h"
#include "db/sqlite.h"
#include "service/util.h"
#include "service/archive.h"
#include "service/sensor.h"
#include "service/sync.h"
#include "service/wunder.h"
#include "worker.h"

#define signo(i)	(SIGRTMIN + (i))

struct worker
{
	int w_signo;			/* Signal number */
	struct itimerspec w_itimer;	/* Interval timer */
	int (*w_action) (void);		/* Action on timer signal */
	int (*w_destroy) (void);

	timer_t w_timer;
	pthread_t w_thread;		/* Thread id */
	int w_failures;			/* Number of failures */
};

static int startup = 1;

static volatile sig_atomic_t shutdown_pending;
static volatile sig_atomic_t hangup_pending;

static struct worker threads[4];	/* Daemon threads */
static size_t threads_nel;		/* Number of elements */

static void
sig_default(sigset_t *set)
{
	(void) sigemptyset(set);
	(void) sigaddset(set, SIGHUP);
	(void) sigaddset(set, SIGTERM);
}

static int
sigtimer_create(int signo, struct itimerspec *it, timer_t *timer)
{
	int errsv;
	struct sigevent se;

	se.sigev_notify = SIGEV_SIGNAL;
	se.sigev_signo = signo;
	se.sigev_value.sival_ptr = timer;

	/* Adjust start value */
	if (it->it_value.tv_sec == 0 && it->it_value.tv_nsec == 0) {
		it->it_value.tv_nsec = 100;
	}

	if (timer_create(CLOCK_MONOTONIC, &se, timer) == -1) {
		syslog(LOG_ERR, "timer_create: %m");
		return -1;
	}
	if (timer_settime(*timer, 0, it, NULL) == -1) {
		syslog(LOG_ERR, "timer_settime: %m");
		goto error;
	}

	return 0;

error:
	errsv = errno;
	(void) timer_delete(&timer);

	errno = errsv;
	return -1;
}

static void *
sigthread_main(void *arg)
{
	int errsv;
	struct worker *dt;
	sigset_t set;
	timer_t timer;

	dt = (struct worker *) arg;

	/* Blocked signals */
	(void) sigemptyset(&set);
	(void) sigaddset(&set, dt->w_signo);

	/* Create timer */
	if (sigtimer_create(dt->w_signo, &dt->w_itimer, &timer) == -1) {
		goto error;
	}

	/* Signal notifications */
	while (!shutdown_pending && !hangup_pending) {
		int ret;
		siginfo_t info;

		ret = sigwaitinfo(&set, &info);
		if (ret == -1) {
			syslog(LOG_ERR, "sigwaitinfo: %m");
			goto error;
		} else {
			if (hangup_pending || shutdown_pending) {
				/* Stop requested */
			} else {
				ret = dt->w_action();
			}
		}
	}

	(void) timer_delete(&timer);

	/* Cleanup */
	if (dt->w_destroy() == -1) {
		return NULL;
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
		syslog(LOG_ERR, "pthread_create: %m");
		return -1;
	}

	return 0;
}

static int
sigthread_kill(struct worker *dt)
{
	int ret;

	ret = 0;

	/* Kill thread (use timer signal) */
	if (pthread_kill(dt->w_thread, dt->w_signo) == -1) {
		ret = -1;
	} else {
		void *th_res;

		if (pthread_join(dt->w_thread, &th_res) == -1) {
			ret = -1;
		}
	}

	return ret;
}

static long long
timespec_ms(struct timespec *ts)
{
	return ts->tv_sec * 1000 + ts->tv_nsec / 1000000;
}


static size_t
nloops_count()
{
	size_t i;
	long long step;
	long long duration;

	/* Sensor interval */
	step = timespec_ms(&threads[1].w_itimer.it_interval);

	/* Longest duration */
	duration = 0;

	for (i = 2; i < threads_nel; i++) {
		long long d = timespec_ms(&threads[i].w_itimer.it_interval);

		if (duration < d) {
			duration = d;
		}
	}

	return divup(duration, step);
}

static int
threads_init(void)
{
	int i;

	threads_nel = 2;

	if (confp->sync.enabled) {
		threads_nel++;
	}
	if (confp->wunder.enabled) {
		threads_nel++;
	}

	for (i = 0; i < threads_nel; i++) {
		threads[i].w_signo = 0;
		threads[i].w_thread = (pthread_t) -1;
	}

	i = 0;

	/* Time synchronization */
	if (confp->sync.enabled) {
		if (sync_init(&threads[i].w_itimer) == -1) {
			goto error;
		}
		threads[i].w_signo = signo(i);
		threads[i].w_action = sync_timer;
		threads[i].w_destroy = sync_destroy;

		i++;
	} else {
		syslog(LOG_WARNING, "Console time synchronization disabled");
	}

	/* Sensor */
	if (sensor_init(&threads[i].w_itimer) == -1) {
		goto error;
	}
	threads[i].w_signo = signo(i);
	threads[i].w_action = sensor_timer;
	threads[i].w_destroy = sensor_destroy;

	i++;

	/* Archive */
	if (archive_init(&threads[i].w_itimer) == -1) {
		goto error;
	}
	threads[i].w_signo = signo(i);
	threads[i].w_action = archive_timer;
	threads[i].w_destroy = archive_destroy;

	i++;

	/* Wunderground */
	if (confp->wunder.enabled) {
		if (wunder_init(&threads[i].w_itimer) == -1) {
			goto error;
		}

		threads[i].w_signo = signo(i);
		threads[i].w_action = wunder_timer;
		threads[i].w_destroy = wunder_destroy;

		i++;
	}

	return 0;

error:
	// TODO: cleanup
	return -1;
}

static int
threads_start(void)
{
	size_t i;
	sigset_t set;

	/* Block all signals */
	(void) sigemptyset(&set);

	for (i = 0; i < threads_nel; i++) {
		(void) sigaddset(&set, threads[i].w_signo);
	}

	if (pthread_sigmask(SIG_BLOCK, &set, NULL) == -1) {
		return -1;
	}

	/* Create threads */
	for (i = 0; i < threads_nel; i++) {
		struct worker *dt = &threads[i];

		if (sigthread_create(dt) == -1) {
			syslog(LOG_ERR, "pthread_create: %m");
			return -1;
		}
	}

	syslog(LOG_INFO, "%zd threads running", threads_nel);

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

		if (dt->w_thread != (pthread_t) -1) {
			if (sigthread_kill(dt) == -1) {
				ret = -1;
			}

			dt->w_thread = (pthread_t) -1;
		}
	}

	return ret;
}

static int
worker_destroy(void)
{
	int ret = 0;

	threads_kill();
	syslog(LOG_INFO, "Threads stopped");

	/* Shutdown requested */
	if (shutdown_pending) {
		/* Unlink shared board */
		if (board_unlink() == -1) {
			ret = -1;
		}

		syslog(LOG_INFO, "Resources released");
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

	sig_default(&set);

	/* Open device */
	if (drv_init() == -1) {
		return -1;
	}
	if (threads_init() == -1) {
		goto error;
	}

	/* Startup initialization */
	if (startup) {
		ssize_t sz;
		size_t nloops;
		size_t nar;

		if (pthread_sigmask(SIG_BLOCK, &set, NULL) == -1) {
			syslog(LOG_ERR, "pthread_sigmask: %m");
			goto error;
		}

		nloops = nloops_count();
		nar = 1;

		if ((sz = board_open(O_CREAT, nloops, nar)) == -1) {
			syslog(LOG_ERR, "board_open: %m");
			goto error;
		}

		startup = 0;

		syslog(LOG_INFO, "Allocated %zdkB for shared board", sz / 1024);
	}

	/* Start all workers */
	if (threads_start() == -1) {
		syslog(LOG_ERR, "threads_start: %m");
		goto error;
	}

	/* Wait for events */
	while (!shutdown_pending && !hangup_pending) {
		int ret;
		siginfo_t info;

		/* Signals to wait for */
		ret = sigwaitinfo(&set, &info);
		if (ret == -1) {
			syslog(LOG_ERR, "sigwaitinfo: %m");
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
		syslog(LOG_ERR, "worker_destroy: %m");
	}

	return 0;

error:
	errsv = errno;
	(void) worker_destroy();

	errno = errsv;
	return -1;
}
