#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <pthread.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <syslog.h>
#ifdef DEBUG
#include <stdio.h>
#endif

#include "libws/log.h"

#include "board.h"
#include "conf.h"
#include "db/sqlite.h"
#include "service/util.h"
#include "service/archive.h"

#define ARCHIVE_INTERVAL 300			/* default archive interval */

static enum ws_driver driver;			/* driver */
static int freq;						/* archive frequency */
static int hw_archive;					/* hardware archive */

static ssize_t
board_put(struct ws_archive *ar)
{
	ssize_t ret;

	if (board_lock() == -1) {
		csyslog1(LOG_CRIT, "board_lock(): %m");
		goto error;
	}

	if (hw_archive) {
		ret = 1;
	} else {
		ret = ws_aggr(ar, freq);
	}

	if (ret) {
		ws_calc(&ar->data);
		board_push_ar(ar);
	}

	if (board_unlock() == -1) {
		csyslog1(LOG_CRIT, "board_unlock(): %m");
		goto error;
	}

	return ret;

error:
	return -1;
}

int
archive_init(struct itimerspec *it)
{
	driver = confp->station.driver;
	hw_archive = 1;

	/*
	 * Test hardware capabilities first.
	 *
	 * Then use configuration supplied frequency. When not set, use the station
	 * archive time otherwise, when supported by the driver.
	 */
	if (drv_get_itimer(it, WS_ITIMER_ARCHIVE) == -1) {
		if (errno == ENOTSUP) {
			hw_archive = 0;
			itimer_set(it, ARCHIVE_INTERVAL);
		} else {
			csyslog1(LOG_ERR, "drv_get_itimer(): %m");
			goto error;
		}
	}
	if (confp->archive.freq == 0) {
		/* Set above */
	} else {
		itimer_set(it, confp->archive.freq);
	}

	/* Some devices are not reliable, prefer software */
	if (hw_archive) {
		switch (driver) {
#ifdef HAVE_WS23XX
		case WS23XX:
			/* Device archives data at archive time, no aggregation */
			hw_archive = 0;
			break;
#endif
		default:
			break;
		}
	}

#ifdef DEBUG
	printf("archive.freq: %ld\n", it->it_interval.tv_sec);
	printf("archive.delay: %ld\n", it->it_value.tv_sec);
	printf("archive.hardware: %d\n", hw_archive);
#endif

	freq = it->it_interval.tv_sec;

	if (confp->archive.sqlite.enabled) {
		ssize_t ret;
		struct ws_archive arbuf;

		/* Initialize database */
		if (sqlite_init() == -1) {
			goto error;
		}

		/* Load last archive from database */
		ret = sqlite_select_last(&arbuf, 1);
		if (ret == -1) {
			goto error;
		} else if (ret > 0) {
//			time_t last;
//			time_t interval;
//
//			last = arbuf.time;
//			interval = it->it_interval.tv_sec;

//			/* Fetch missing archives from device */
//			drv_archive_open();
//			drv_archive_next(&arbuf);
//
//			while (arbuf.time > last) {
//
//				drv_archive_next(&arbuf);
//			}

			// TODO: take care that this puts entries in shared map
			// and that may trigger wunder updates
//			if (board_put(&arbuf, 1, 0) == -1) {
//				goto error;
//			}
		}
	}

	syslog(LOG_INFO, "%s: done", __func__);

	return 0;

error:
	return -1;
}

int
archive_main(void)
{
	ssize_t arsz;
	struct ws_archive arbuf;

#ifdef DEBUG
	printf("ARCHIVE: reading\n");
#endif

	if (hw_archive) {
		if (drv_get_archive(&arbuf, 1) == -1) {
			csyslog1(LOG_ERR, "drv_get_archive(): %m");
			goto error;
		}
	}

	/* Update board */
	arsz = board_put(&arbuf);
	if (arsz == -1) {
		goto error;
	} else if (arsz > 0) {
#ifdef DEBUG
		printf("ARCHIVE read: %.2f°C %hhu%%\n", arbuf.data.temp, arbuf.data.humidity);
#endif

		/* Save to database */
		if (confp->archive.sqlite.enabled) {
			if (sqlite_insert(&arbuf, 1) == -1) {
				goto error;
			}
		}
	} else {
#ifdef DEBUG
		printf("ARCHIVE no read\n");
#endif
	}

	return 0;

error:
	return -1;
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
