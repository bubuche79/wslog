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

#include "sqlite.h"
#include "board.h"
#include "wslogd.h"
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
	if (confp->sqlite.freq == 0) {
		/* Unreliable hardware */
		switch (driver)
		{
#ifdef HAVE_WS23XX
		case WS23XX:
			ret = -1;
			break;
#endif
		default:
			break;
		}

		// TODO
	} else {
		freq = confp->sqlite.freq;
	}

	/* Flush buffer, if configured */
//	if (confp->sqlite.flush_freq > 0) {
//		size_t size = divup(confp->sqlite.flush_freq, confp->sqlite.freq);
//
//		tx_idx = 0;
//		tx_buf = malloc(size * sizeof(*tx_buf));
//		if (tx_buf == NULL) {
//			goto error;
//		}
//	}

	if (sqlite_init() == -1) {
		goto error;
	}

	return 0;

error:
	return ret;
}

int
archive_main(void)
{
	time_t next;
	struct ws_archive ar;

	(void) time(&next);

	if (board_lock() == -1) {
		csyslog1(LOG_CRIT, "board_lock(): %m");
		return -1;
	}

	/* Compute metrics since last archive */
	// TODO
	ar.time = next;
	ar.interval = freq;
	memcpy(&ar.data, board_peek(0), sizeof(ar.data));

	board_push_ar(&ar);

	if (board_unlock() == -1) {
		csyslog1(LOG_CRIT, "board_lock(): %m");
		return -1;
	}

	last = next;

	/* Save to database */
	if (sqlite_write(&ar) == -1) {
		return -1;
	}

	return 0;
}

int
archive_destroy(void)
{
	if (sqlite_destroy() == -1) {
		return -1;
	}

	return 0;
}
