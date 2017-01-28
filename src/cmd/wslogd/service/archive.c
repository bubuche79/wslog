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
#include "service/service.h"

static time_t last;
static enum ws_driver driver;
static int freq;
//static size_t tx_idx;
//static struct ws_archive *tx_buf;

int
archive_init(void)
{
	int ret;
//	int hw_archive;

	(void) time(&last);
	driver = confp->station.driver;

	ret = 0;

	/*
	 * Use configuration supplied frequency. When not set, use the station
	 * archive time otherwise, when supported by the driver.
	 */
	if (confp->archive.freq == 0) {
		struct itimerspec buf;

		if (drv_get_itimer(&buf, WS_ITIMER_ARCHIVE) == -1) {
			goto error;
		}

		freq = buf.it_interval.tv_sec;
	} else {
		freq = confp->archive.freq;
	}

	/* Initialize database */
	if (confp->sqlite.enabled) {
		if (sqlite_init() == -1) {
			goto error;
		}
	}

	return 0;

error:
	return ret;
}

int
archive_main(void)
{
	struct ws_archive ar;

#if DEBUG
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

#if DEBUG
	printf("ARCHIVE read: %.2f°C %hhu%%\n", ar.data.temp, ar.data.humidity);
#endif

	/* Save to database */
	if (confp->sqlite.enabled) {
		if (sqlite_insert(&ar) == -1) {
			return -1;
		}
	}

	return 0;
}

int
archive_destroy(void)
{
	if (confp->sqlite.enabled) {
		if (sqlite_destroy() == -1) {
			return -1;
		}
	}

	return 0;
}
