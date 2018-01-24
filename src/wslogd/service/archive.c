#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <pthread.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <syslog.h>

#include "board.h"
#include "conf.h"
#include "db/sqlite.h"
#include "service/util.h"
#include "service/archive.h"

#define ARCHIVE_INTERVAL 300		/* Default archive interval */
#define AR_LEN 64			/* Archive buffer size */

static enum ws_driver driver;		/* Driver */
static int freq;			/* Archive frequency */
static int hw_archive;			/* Hardware archive */
static time_t current;			/* Last known console archive record */

static int
board_put(struct ws_archive *ar, size_t nel)
{
	size_t i;
	ssize_t ret;

	if (board_lock() == -1) {
		syslog(LOG_CRIT, "board_lock: %m");
		goto error;
	}

	for (i = 0; i < nel; i++) {
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
		syslog(LOG_CRIT, "board_unlock: %m");
		goto error;
	}

	return 0;

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
			goto error;
		}
	}
	if (confp->archive.freq == 0) {
		/* Set above */
	} else {
		// TODO: mismatch conf/console
		itimer_set(it, confp->archive.freq);
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

#ifdef DEBUG
	syslog(LOG_INFO, "archive.freq: %ld\n", it->it_interval.tv_sec);
	syslog(LOG_INFO, "archive.delay: %ld\n", it->it_value.tv_sec);
	syslog(LOG_INFO, "archive.hardware: %d\n", hw_archive);
#endif

	freq = it->it_interval.tv_sec;

	if (confp->archive.sqlite.enabled) {
		ssize_t ret;
		struct ws_archive arbuf[AR_LEN];

		/* Initialize database */
		if (sqlite_init() == -1) {
			goto error;
		}

		/* Load last archive from database */
		ret = sqlite_select_last(arbuf, 1);
		if (ret == -1) {
			goto error;
		} else if (ret > 0) {
			ssize_t sz;

			current = arbuf[0].time;

			do {
				sz = drv_get_archive(arbuf, AR_LEN, current);
				if (sz == -1) {
					goto error;
				} else if (sz > 0) {
					if (board_put(arbuf, sz) == -1) {
						goto error;
					}

					/* Save to database */
					if (confp->archive.sqlite.enabled) {
						if (sqlite_insert(arbuf, sz) == -1) {
							goto error;
						}
					}

					/* Next start point */
					current = arbuf[sz - 1].time;
				}
			} while (sz == AR_LEN);
		}
	} else {
		// TODO: pick last archive record timestamp (idem in final else above)
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

	if (hw_archive) {
		if (drv_get_archive(&arbuf, 1, current) == -1) {
			goto error;
		}
	}

	/* Update board */
	arsz = board_put(&arbuf, 1);
	if (arsz == -1) {
		goto error;
	} else if (arsz > 0) {
#ifdef DEBUG
		syslog(LOG_DEBUG, "Record: %.1f°C %hhu%% %.1fhPa",
				arbuf.temp, arbuf.humidity, arbuf.barometer);
#endif

		/* Save to database */
		if (confp->archive.sqlite.enabled) {
			if (sqlite_insert(&arbuf, 1) == -1) {
				goto error;
			}
		}

		current = arbuf.time;
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
