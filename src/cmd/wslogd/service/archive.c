#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <pthread.h>
#include <stddef.h>
#include <string.h>

#include <time.h>
#include <syslog.h>
#ifdef DEBUG
#include <stdio.h>
#endif

#include "libws/log.h"

#include "board.h"
#include "wslogd.h"
#include "db/sqlite.h"
#include "service/util.h"
#include "service/archive.h"

int
archive_init(struct itimerspec *it)
{
	/*
	 * Use configuration supplied frequency. When not set, use the station
	 * archive time otherwise, when supported by the driver.
	 */
	if (confp->archive.freq == 0) {
		if (drv_get_itimer(it, WS_ITIMER_ARCHIVE) == -1) {
			goto error;
		}
	} else {
		itimer_set(it, confp->archive.freq);
	}

#ifdef DEBUG
	printf("archive.freq: %ld\n", it->it_interval.tv_sec);
	printf("archive.delay: %ld\n", it->it_value.tv_sec);
#endif

	/* Initialize database */
	if (confp->archive.sqlite.enabled) {
		if (sqlite_init() == -1) {
			goto error;
		}
	}

	return 0;

error:
	csyslog1(LOG_ERR, "archive_init(): %m");
	return -1;
}

int
archive_main(void)
{
	struct ws_archive ar;

#ifdef DEBUG
	printf("ARCHIVE: reading\n");
#endif

	if (drv_get_archive(&ar, 1) == -1) {
		return -1;
	}

	/* Update board */
	if (board_lock() == -1) {
		csyslog1(LOG_CRIT, "board_lock(): %m");
		return -1;
	}

	board_push_ar(&ar);

	if (board_unlock() == -1) {
		csyslog1(LOG_CRIT, "board_unlock(): %m");
		return -1;
	}

#ifdef DEBUG
	printf("ARCHIVE read: %.2f°C %hhu%%\n", ar.data.temp, ar.data.humidity);
#endif

	/* Save to database */
	if (confp->archive.sqlite.enabled) {
		if (sqlite_insert(&ar) == -1) {
			return -1;
		}
	}

	return 0;
}

int
archive_destroy(void)
{
	if (confp->archive.sqlite.enabled) {
		if (sqlite_destroy() == -1) {
			return -1;
		}
	}

	return 0;
}
