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

#ifdef HAVE_WS23XX
#include "driver/ws23xx.h"
#endif
#include "board.h"
#include "wslogd.h"
#include "service/service.h"

static time_t last;
static enum ws_driver driver;

int
archive_init(void)
{
	int ret;
	int hw_archive;

	(void) time(&last);
	driver = confp->station.driver;

	ret = 0;

	/*
	 * Use configuration supplied frequency. When not set, use the station
	 * archive time otherwise, when supported by the driver.
	 */
	if (confp->sqlite.freq == 0) {
		hw_archive = 1;
	} else {
		hw_archive = 0;

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

		/* Required configuration not set */
		if (ret == -1) {
			syslog(LOG_ERR, "sqlite.freq shall be set");
			goto error;
		}
	}

	// TODO: set timespec

	if (hw_archive) {

	}

	return 0;

error:
	return ret;
}

int
archive_main(void)
{
	int dirty;
	time_t next;

	struct ws_archive ar;

	(void) time(&next);

	if (board_lock() == -1) {
		return -1;
	}

	/* Compute metrics since last archive */
	dirty = 1;

//	for (i = 0; dirty && i < boardp->loop_nel; i++) {
//		const struct ws_loop *p = board_loop_p(i);
//
//		if (last < p->time.tv_sec) {
//			//aggr_update(&aggr, p);
//		} else {
//			/* Already processed */
//			dirty = 0;
//		}
//	}

	// TODO
	ar.time = next;
	ar.interval = 300;
	memcpy(&ar.data, board_peek(0), sizeof(ar.data));

	board_push_ar(&ar);

	if (board_unlock() == -1) {
		return -1;
	}

	/* Finalize aggregate */
	//aggr_finalize(&aggr, &ar, i);

	if (dirty)
		;
	last = next;

	return 0;
}

int
archive_destroy(void)
{
	return 0;
}
