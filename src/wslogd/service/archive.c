#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <pthread.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <syslog.h>

#include "libws/util.h"

#include "board.h"
#include "conf.h"
#include "db/sqlite.h"
#include "service/util.h"
#include "service/archive.h"

#define WSLOG_EPOCH 1514764800		/* Mon, 1 Jan 2018 00:00:00 */
#define ARCHIVE_INTERVAL 600		/* Default archive interval */
#define AR_LEN 64			/* Archive buffer size */

static enum ws_driver driver;		/* Driver */
static int freq;			/* Archive frequency */
static int hw_archive;			/* Hardware archive */
static time_t current;			/* Last known console archive record */

static ssize_t
push_record(struct ws_archive *ar, size_t nel)
{
	size_t i;

	if (board_lock() == -1) {
		syslog(LOG_ERR, "board_lock: %m");
		goto error;
	}

	for (i = 0; i < nel; i++) {
		int ret;

		if (hw_archive) {
			ret = 1;
		} else {
			ret = ws_aggr(&ar[i], freq);
		}

		if (ret) {
			ws_calc(&ar[i]);
			board_push_ar(&ar[i]);
		}
	}

	if (board_unlock() == -1) {
		syslog(LOG_ERR, "board_unlock: %m");
		goto error;
	}

	return nel;

error:
	return -1;
}

static int
bulk_fetch()
{
	ssize_t sz, total;
	struct ws_archive arbuf[AR_LEN];

	/* Start transaction */
	if (sqlite_begin() == -1) {
		return -1;
	}

	total = 0;

	/* Fetch records, and save them into database */
	do {
		sz = drv_get_ar(arbuf, AR_LEN, current);
		if (sz == -1) {
			goto error;
		} else if (sz > 0) {
			if (sqlite_insert(arbuf, sz) == -1) {
				goto error;
			}

			total += sz;

			/* Next start point */
			current = arbuf[sz - 1].time;
		}
	} while (sz == AR_LEN);

	/* Commit */
	if (sqlite_commit() == -1) {
		goto error;
	}

	syslog(LOG_NOTICE, "Fetched %zd missed records", total);

	return 0;

error:
	(void) sqlite_rollback();

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
	if (drv_get_ar_itimer(it) == -1) {
		if (errno == ENOTSUP) {
			hw_archive = 0;
			itimer_setdelay(it, ARCHIVE_INTERVAL, 0);
		} else {
			goto error;
		}
	}
	if (confp->archive.freq == 0) {
		/* Set above */
	} else if (hw_archive && confp->archive.freq != it->it_interval.tv_sec) {
		syslog(LOG_ERR, "Hardware archive differs from archive.freq");
		goto error;
	}

	/* Some devices are not reliable, use software archive */
	if (hw_archive) {
		switch (driver) {
#ifdef HAVE_WS23XX
		case WS23XX:
			/*
			 * All fields are picked at the final archive period. There is no
			 * aggregation to compute average wind speed, for example.
			 */
			hw_archive = 0;
			it->it_value.tv_sec = 0;
			break;
#endif
		default:
			break;
		}
	}

	freq = it->it_interval.tv_sec;

	/*
	 * Records are saved into database.
	 *
	 * Initialize the database handle, load all missed records since last
	 * database update, and adjust internal state variables.
	 */
	if (confp->archive.sqlite.enabled) {
		if (sqlite_init() == -1) {
			goto error;
		}

		/* Use console records */
		if (hw_archive) {
			ssize_t sz;
			struct ws_archive arbuf;

			/* Timestamp of last database record */
			sz = sqlite_select_last(&arbuf, 1);
			if (sz == -1) {
				goto error;
			}

			if (sz > 0) {
				char ftime[20];

				current = arbuf.time;

				localftime_r(ftime, sizeof(ftime), &current, "%F %T");
				syslog(LOG_NOTICE, "Last db record: %s", ftime);
			} else {
				current = WSLOG_EPOCH;
			}

			/* Load console records after that point */
			if (bulk_fetch() == -1) {
				goto error;
			}

			/* Adjust timer delay */
			if (drv_get_ar_itimer(it) == -1) {
				goto error;
			}
		}
	} else {
		/* Start from now on */
		time(&current);
	}

#ifdef DEBUG
	syslog(LOG_INFO, "archive.freq=%ld\n", it->it_interval.tv_sec);
	syslog(LOG_INFO, "archive.delay=%ld\n", it->it_value.tv_sec);
	syslog(LOG_INFO, "archive.hardware=%d\n", hw_archive);
#endif

	syslog(LOG_INFO, "Archive service ready");

	return 0;

error:
	return -1;
}

int
archive_sig_timer(struct ws_archive *ar)
{
	ssize_t sz;

	/* Device archive */
	if (hw_archive) {
		if ((sz = drv_get_ar(ar, 1, current)) == -1) {
			goto error;
		}

		if (sz > 0) {
			current = ar->time;
		}
	} else {
		ar->wl_mask = 0;
		sz = 1;
	}

	/* Update board */
	sz = push_record(ar, sz);
	if (sz == -1) {
		goto error;
	} else if (sz > 0) {
#ifdef DEBUG
		char ftime[20];

		localftime_r(ftime, sizeof(ftime), &ar->time, "%F %T");
		syslog(LOG_DEBUG, "Record: %s %.1f°C %hhu%% %.1fhPa",
				ftime, ar->temp, ar->humidity, ar->barometer);
#endif

		/* Save to database */
		if (confp->archive.sqlite.enabled) {
			if (sqlite_insert(ar, sz) == -1) {
				goto error;
			}
		}
	} else {
		syslog(LOG_NOTICE, "No archive fetched");
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
