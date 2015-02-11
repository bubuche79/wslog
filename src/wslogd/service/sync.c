#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <syslog.h>

#include "conf.h"
#include "service/util.h"
#include "service/sync.h"

static long freq;		/* Sync. frequency */

int
sync_init(struct itimerspec *it)
{
	freq = confp->sync.freq;
	if (freq == 0) {
		freq = 86400;
	}

	itimer_set(it, freq);

	/* Synchronize at startup */
	if (sync_timer() == -1) {
		goto error;
	}

#ifdef DEBUG
	syslog(LOG_INFO, "sync.freq: %ld\n", it->it_interval.tv_sec);
#endif

	syslog(LOG_INFO, "Time synchronization service ready");

	return 0;

error:
	return -1;
}

int
sync_timer(void)
{
	time_t dev_time;
	time_t now, diff;

	if (drv_time(&dev_time) == -1) {
		goto error;
	}

	time(&now);
	diff = dev_time - now;

	/* Beyond limit, adjust time */
	if (labs(diff) > confp->sync.max_drift) {
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
