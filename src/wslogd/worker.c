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
#include <string.h>
#include <time.h>

#include "libws/defs.h"

#include "board.h"
#include "conf.h"
#include "db/sqlite.h"
#include "service/util.h"
#include "service/archive.h"
#include "service/sensor.h"
#include "service/ic.h"
#include "service/sync.h"
#include "service/wunder.h"
#include "worker.h"

#define sigrtno(i)	(SIGRTMIN + (i))

struct worker
{
	int signo;				/* Signal number */

	int (*wmain)(void);			/* Simple thread runner */
	int (*wdestroy)(void);			/* Destroy */

	int flags;				/* Event flags */
	struct itimerspec itimer;		/* Interval timer */
	int (*ef_timer)(void);			/* Action on timer signal */
	int (*ef_rt)(const struct ws_loop *);	/* Action on real-time sensor data */
	int (*ef_ar)(const struct ws_archive *);	/* Action on archive data*/

	struct ws_loop rt;
	struct ws_archive ar;

	pthread_t tid;				/* Thread id */
	int failures;				/* Number of failures */
};

static int startup = 1;

static volatile sig_atomic_t shutdown_pending;
static volatile sig_atomic_t hangup_pending;

static struct worker threads[8];	/* Daemon threads */
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
sigtimer_main(void *arg)
{
	int errsv;
	struct worker *dt;
	sigset_t set;
	timer_t timer;

	dt = (struct worker *) arg;

	/* Blocked signals */
	(void) sigemptyset(&set);
	(void) sigaddset(&set, dt->signo);

	/* Create timer */
	if (dt->ef_timer) {
		if (sigtimer_create(dt->signo, &dt->itimer, &timer) == -1) {
			goto error;
		}
	}

	/* Signal notifications */
	while (!shutdown_pending && !hangup_pending) {
		siginfo_t info;

		if (sigwaitinfo(&set, &info) == -1) {
			syslog(LOG_ERR, "sigwaitinfo: %m");
			goto error;
		} else {
			if (hangup_pending || shutdown_pending) {
				/* Stop requested */
			} else {
				int dt_ret = -1;
				int si_code = info.si_code;

				if (si_code == SI_TIMER) {
					dt_ret = dt->ef_timer();
				} else if (si_code == SI_QUEUE) {
					int ef_code = info.si_value.sival_int;

					if (ef_code == SRV_EVENT_RT) {
						dt_ret = dt->ef_rt(&dt->rt);
					} else if (ef_code == SRV_EVENT_AR) {
						dt_ret = dt->ef_ar(&dt->ar);
					}
				}

				if (dt_ret == -1) {
					dt->failures++;
				}
			}
		}
	}

	if (dt->ef_timer) {
		(void) timer_delete(&timer);
	}

	/* Cleanup */
	if (dt->wdestroy() == -1) {
		return NULL;
	}
	return NULL;

error:
	errsv = errno;
	(void) timer_delete(&timer);
	(void) dt->wdestroy();

	errno = errsv;
	return NULL;
}

static void *
sigthread_main(void *arg)
{
	struct worker *dt;

	dt = (struct worker *) arg;

	while (!shutdown_pending && !hangup_pending) {
		if (dt->wmain() == -1) {
			dt->failures++;
		}
	}

	if (dt->wdestroy() == -1) {
		return NULL;
	}
	return NULL;
}

static int
sigthread_create(struct worker *dt)
{
	void *(*func) (void *);

	if (dt->wmain) {
		func = sigthread_main;
	} else {
		func = sigtimer_main;
	}

	if (pthread_create(&dt->tid, NULL, func, dt) == -1) {
		syslog(LOG_ERR, "pthread_create: %m");
		return -1;
	}

	return 0;
}

static int
sigthread_kill(struct worker *dt)
{
	void *th_res;

	/* Kill thread */
	if (dt->wmain) {
		if (pthread_cancel(dt->tid) == -1) {
			syslog(LOG_ERR, "pthread_cancel: %m");
			goto error;
		}
	} else {
		if (pthread_kill(dt->tid, dt->signo) == -1) {
			syslog(LOG_ERR, "pthread_kill %d: %m", dt->signo);
			goto error;
		}
	}

	/* Wait for thread to complete */
	if (pthread_join(dt->tid, &th_res) == -1) {
		syslog(LOG_ERR, "pthread_join: %m");
		goto error;
	}

	return 0;

error:
	return -1;
}

static int
sigevent_sensor()
{
	int ret;
	struct ws_loop rt;

	ret = sensor_sig_timer(&rt);

	/* Notify other threads */
	if (ret == 0) {
		int i;

		for (i = 0; i < threads_nel; i++) {
			struct worker *dt = &threads[i];

			if (dt->flags & SRV_EVENT_RT) {
				union sigval val = { SRV_EVENT_RT };

				memcpy(&dt->rt, &rt, sizeof(rt));
				pthread_sigqueue(dt->tid, dt->signo, val);
			}
		}
	}

	return ret;
}

static int
sigevent_archive()
{
	int ret;
	struct ws_archive ar;

	ret = archive_sig_timer(&ar);

	/* Notify other threads */
	if (ret == 0) {
		int i;

		for (i = 0; i < threads_nel; i++) {
			struct worker *dt = &threads[i];

			if (threads[i].flags & SRV_EVENT_AR) {
				union sigval val = { SRV_EVENT_AR };

				memcpy(&dt->ar, &ar, sizeof(ar));
				pthread_sigqueue(dt->tid, dt->signo, val);
			}
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
	step = timespec_ms(&threads[0].itimer.it_interval);

	/* Longest duration */
	duration = 0;

	for (i = 1; i < threads_nel; i++) {
		long long d = timespec_ms(&threads[i].itimer.it_interval);

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

	threads_nel = 3;
	memset(threads, 0, sizeof(threads));

	if (confp->sync.enabled) {
		threads_nel++;
	}
	if (confp->stat_ic.enabled) {
		threads_nel++;
	}
	if (confp->wunder.enabled) {
		threads_nel++;
	}

	for (i = 0; i < threads_nel; i++) {
		threads[i].tid = (pthread_t) -1;
	}

	i = 0;

	/* Sensor */
	if (sensor_init(&threads[i].itimer) == -1) {
		goto error;
	}
	threads[i].signo = sigrtno(i);
	threads[i].ef_timer = sigevent_sensor;
	threads[i].wdestroy = sensor_destroy;

	i++;

	/* Archive */
	if (archive_init(&threads[i].itimer) == -1) {
		goto error;
	}
	threads[i].signo = sigrtno(i);
	threads[i].ef_timer = sigevent_archive;
	threads[i].wdestroy = archive_destroy;

	i++;

	/* Time synchronization */
	if (confp->sync.enabled) {
		if (sync_init(&threads[i].flags, &threads[i].itimer) == -1) {
			goto error;
		}
		threads[i].signo = sigrtno(i);
		threads[i].ef_timer = sync_sig_timer;
		threads[i].wdestroy = sync_destroy;

		i++;
	} else {
		syslog(LOG_WARNING, "Console time synchronization disabled");
	}

	/* StatIC */
	if (confp->stat_ic.enabled) {
		if (ic_init(&threads[i].flags, &threads[i].itimer) == -1) {
			goto error;
		}

		threads[i].signo = sigrtno(i);
		threads[i].ef_ar = ic_sig_ar;
		threads[i].ef_rt = ic_sig_rt;
		threads[i].wdestroy = ic_destroy;

		i++;
	}

	/* Wunderground */
	if (confp->wunder.enabled) {
		if (wunder_init(&threads[i].flags, &threads[i].itimer) == -1) {
			goto error;
		}

		threads[i].signo = sigrtno(i);
		threads[i].ef_timer = wunder_sig_timer;
		threads[i].wdestroy = wunder_destroy;

		i++;
	}

	return 0;

error:
	// TODO: cleanup
	return -1;
}

static int
threads_create(void)
{
	size_t i;
	sigset_t set;

	/* Block all signals */
	(void) sigemptyset(&set);

	for (i = 0; i < threads_nel; i++) {
		(void) sigaddset(&set, threads[i].signo);
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

		if (dt->tid != (pthread_t) -1) {
			if (sigthread_kill(dt) == -1) {
				ret = -1;
			}

			dt->tid = (pthread_t) -1;
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
	if (threads_create() == -1) {
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
				syslog(LOG_NOTICE, "Signal HUP received");
				break;
			case SIGTERM:
				shutdown_pending = 1;
				syslog(LOG_NOTICE, "Signal TERM received");
				break;
			default:
				syslog(LOG_ERR, "Signal %d received", ret);
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
