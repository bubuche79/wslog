#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <syslog.h>

#include "conf.h"
#include "service/util.h"
#include "service/sync.h"

static long freq;		/* Sync. frequency, in seconds */
static long max_drift;		/* Max drift, in seconds */
static long panic_drift;	/* Do not synchronize beyond this limit */

int
sync_init(int *flags, struct itimerspec *it)
{
	freq = confp->sync.freq;
	max_drift = confp->sync.max_drift;
	panic_drift = 1000;

	if (freq == 0) {
		freq = 7200;
	}
	if (max_drift == 0) {
		max_drift = 2;
	}

	/*
	 * Delay first synchronization, to make sure that the system clock
	 * is correctly set upon startup when there is no RTC backup battery.
	 */
	itimer_setdelay(it, freq, 300);

#ifdef DEBUG
	syslog(LOG_INFO, "sync.freq=%ld\n", it->it_interval.tv_sec);
#endif

	*flags = SRV_TIMER;

	syslog(LOG_INFO, "Time synchronization service ready");

	return 0;
}

int
sync_sig_timer(void)
{
	time_t dev_time;
	time_t now, diff;

	if (drv_time(&dev_time) == -1) {
		goto error;
	}

	time(&now);
	diff = dev_time - now;

	/* Beyond limit, adjust time */
	if (labs(diff) > panic_drift) {
		syslog(LOG_CRIT, "Time beyond limit (%lds)", diff);
	} else if (labs(diff) > confp->sync.max_drift) {
		if (drv_settime(now) == -1) {
			goto error;
		}

		syslog(LOG_INFO, "Console time adjusted (%lds)", diff);
	}

	return 0;

error:
	return -1;
}

int
sync_destroy(void)
{
	return 0;
}
