#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <pthread.h>
#include <sys/file.h>
#include <errno.h>
#include <syslog.h>

#include "defs/std.h"

#include "libws/util.h"
#include "libws/vantage/util.h"
#include "libws/vantage/vantage.h"

#include "driver/vantage.h"
#include "conf.h"

#define ARCHIVE_DELAY	15		/* Delay before fetching new record */

static int fd;				/* Device file */
static pthread_mutex_t mutex;		/* Thread locking */

static enum vantage_type wrd;		/* Console type */
static struct vantage_cfg cfg;		/* Console configuration */

static int
vantage_lock(int fd)
{
	int locked;

	/* Lock device */
	locked = 0;

	if (flock(fd, LOCK_EX) == -1) {
		syslog(LOG_ERR, "flock (ex): %m");
		goto error;
	}

	locked++;

	if (pthread_mutex_lock(&mutex) == -1) {
		syslog(LOG_ERR, "pthread_mutex_lock: %m");
		goto error;
	}

	locked++;

	/* Wakeup console */
	if (vantage_wakeup(fd) == -1) {
		syslog(LOG_ERR, "vantage_wakeup: %m");
		goto error;
	}

	return 0;

error:
	if (locked > 1) {
		(void) pthread_mutex_unlock(&mutex);
	}
	if (locked > 0) {
		(void) flock(fd, LOCK_UN);
	}

	return -1;
}

static int
vantage_unlock(int fd)
{
	if (pthread_mutex_unlock(&mutex) == -1) {
		syslog(LOG_ERR, "pthread_mutex_unlock: %m");
		goto error;
	}
	if (flock(fd, LOCK_UN) == -1) {
		syslog(LOG_ERR, "flock (un): %m");
		goto error;
	}

	return 0;

error:
	return -1;
}

#if 0
static time_t
vantage_rec_last(int fd)
{
	ssize_t sz;
	time_t after;
	struct vantage_dmp buf;

	/*
	 * Hopefully, there is one archive record in the last archive record
	 * period. If this is not the case, try again with max archive record
	 * delay (120 minutes). And then fall back on current timestamp (there
	 * is no archive record).
	 */
	after = time(NULL) - cfg.ar_period * 60;

	do {
		if ((sz = vantage_dmpaft(fd, &buf, 1, after)) == -1) {
			syslog(LOG_ERR, "vantage_dmpaft: %m");
			goto error;
		}

		if (sz > 0) {
			after = buf.time;
		}
		// TODO
	} while (sz > 0);

	return after;

error:
	return -1;
}
#endif

static void
conv_ar_dmp(struct ws_archive *p, const struct vantage_dmp *d)
{
	p->time = d->time;
	p->interval = cfg.ar_period * 60;
	p->wl_mask = 0;

	if (d->temp != INT16_MAX) {
		p->wl_mask |= WF_TEMP;
		p->temp = vantage_temp(d->temp, 1);
	}
	if (d->lo_temp != INT16_MAX) {
		p->wl_mask |= WF_LO_TEMP;
		p->lo_temp = vantage_temp(d->lo_temp, 1);
	}
	if (d->hi_temp != INT16_MAX) {
		p->wl_mask |= WF_HI_TEMP;
		p->hi_temp = vantage_temp(d->hi_temp, 1);
	}
	if (d->humidity != UINT8_MAX) {
		p->wl_mask |= WF_HUMIDITY;
		p->humidity = d->humidity;
	}
	if (d->barometer != 0) {
		p->wl_mask |= WF_BAROMETER;
		p->barometer = vantage_pressure(d->barometer, 3);
	}

	if (d->in_temp != INT16_MAX) {
		p->wl_mask |= WF_IN_TEMP;
		p->in_temp = vantage_temp(d->in_temp, 1);
	}
	if (d->in_humidity != UINT8_MAX) {
		p->wl_mask |= WF_IN_HUMIDITY;
		p->in_humidity = d->in_humidity;
	}

	if (d->avg_wind_speed != UINT8_MAX) {
		p->wl_mask |= WF_WIND_SPEED;
		p->avg_wind_speed = vantage_speed(d->avg_wind_speed);
	}
	if (d->main_wind_dir != UINT8_MAX) {
		p->wl_mask |= WF_WIND_DIR;
		p->avg_wind_dir= d->main_wind_dir;
	}
	if (d->hi_wind_speed != UINT8_MAX) {
		p->wl_mask |= WF_HI_WIND_SPEED;
		p->hi_wind_speed = vantage_speed(d->hi_wind_speed);
	}
	if (d->hi_wind_dir != UINT8_MAX) {
		p->wl_mask |= WF_HI_WIND_DIR;
		p->hi_wind_dir = d->hi_wind_dir;
	}

	p->wl_mask |= WF_RAIN_FALL|WF_HI_RAIN_RATE;
	p->rain_fall = vantage_rain(d->rain, cfg.sb_rain_cup);
	p->hi_rain_rate = vantage_rain(d->hi_rain_rate, cfg.sb_rain_cup);
}

static void
conv_curr_lps(struct ws_loop *p, const struct vantage_loop *d)
{
	time(&p->time);
	p->wl_mask = 0;

	if (d->temp != INT16_MAX) {
		p->wl_mask |= WF_TEMP;
		p->temp = vantage_temp(d->temp, 1);
	}
	if (d->humidity != UINT8_MAX) {
		p->wl_mask |= WF_HUMIDITY;
		p->humidity = d->humidity;
	}
	if (d->barometer != 0) {
		p->wl_mask |= WF_BAROMETER;
		p->barometer = vantage_pressure(d->barometer, 3);
	}

	if (d->in_temp != INT16_MAX) {
		p->wl_mask |= WF_IN_TEMP;
		p->in_temp = vantage_temp(d->in_temp, 1);
	}
	if (d->in_humidity != UINT8_MAX) {
		p->wl_mask |= WF_IN_HUMIDITY;
		p->in_humidity = d->in_humidity;
	}

	if (d->wind_dir != 0) {
		p->wl_mask |= WF_WIND_SPEED | WF_WIND_DIR;
		p->wind_speed = vantage_speed(d->wind_speed);
		p->wind_dir= d->wind_dir;
	}

	if (d->dew_point != INT8_MAX) {
		p->wl_mask |= WF_DEW_POINT;
		p->dew_point = vantage_temp(d->dew_point, 0);
	}
	if (d->wind_chill != INT8_MAX) {
		p->wl_mask |= WF_WINDCHILL;
		p->windchill = vantage_temp(d->wind_chill, 0);
	}
	if (d->heat_index != INT8_MAX) {
		p->wl_mask |= WF_HEAT_INDEX;
		p->windchill = vantage_temp(d->heat_index, 0);
	}

	p->wl_mask |= WF_RAIN_FALL|WF_HI_RAIN_RATE;
	p->rain_day = vantage_rain(d->daily_rain, cfg.sb_rain_cup);
	p->rain_rate = vantage_rain(d->rain_rate, cfg.sb_rain_cup);
}

int
vantage_init(void)
{
	const char *tty = confp->driver.vantage.tty;

	if (pthread_mutex_init(&mutex, NULL) == -1) {
		syslog(LOG_ERR, "pthread_mutex_init: %m");
		goto error;
	}

	/* Open device */
	if ((fd = vantage_open(tty)) == -1) {
		syslog(LOG_ERR, "vantage_open %s: %m", tty);
		goto error;
	}

	if (vantage_lock(fd) == -1) {
		goto error;
	}

	/* Read console configuration */
	if (vantage_wrd(fd, &wrd) == -1) {
		syslog(LOG_ERR, "vantage_wrd: %m");
		goto error;
	}
	if (vantage_ee_cfg(fd, &cfg) == -1) {
		syslog(LOG_ERR, "vantage_ee_cfg: %m");
		goto error;
	}

	/* Release */
	if (vantage_unlock(fd) == -1) {
		goto error;
	}

	syslog(LOG_NOTICE, "%s initialized", vantage_type_str(wrd));

	return 0;

error:
	if (fd != -1) {
		(void) vantage_close(fd);
	}

	return -1;
}

int
vantage_destroy(void)
{
	if (vantage_close(fd) == -1) {
		syslog(LOG_ERR, "vantage_close: %m");
		goto error;
	}

	return 0;

error:
	return -1;
}

int
vantage_get_itimer(struct itimerspec *it, enum ws_timer type)
{
	if (WS_ITIMER_LOOP == type) {
		it->it_interval.tv_sec = 2;
		it->it_interval.tv_nsec = 500*1000;
		it->it_value.tv_sec = 0;
		it->it_value.tv_nsec = 0;
	} else if (WS_ITIMER_ARCHIVE == type) {
//		char ftime[20];
//		size_t current;
//
//		/* Compute last archive record timestamp */
//		if ((current = vantage_rec_last(fd)) == -1) {
//			goto error;
//		}
//
//		localftime_r(ftime, sizeof(ftime), &current, "%F %R");
//		syslog(LOG_NOTICE, "last archive: %s", ftime);

		// TODO: check that archives are generated at rounded timestamps
		ws_itimer_delay(it, cfg.ar_period * 60, ARCHIVE_DELAY);
	} else {
		errno = EINVAL;
		goto error;
	}

	return 0;

error:
	return -1;
}

int
vantage_get_loop(struct ws_loop *p)
{
	struct vantage_loop lbuf;

	if (vantage_lock(fd) == -1) {
		goto error;
	}

	if (vantage_lps(fd, LPS_LOOP2, &lbuf, 1) == -1) {
		syslog(LOG_ERR, "vantage_lps: %m");
		goto error;
	}

	if (vantage_unlock(fd) == -1) {
		goto error;
	}

	conv_curr_lps(p, &lbuf);

	return 0;

error:
	(void) vantage_unlock(fd);

	return -1;
}

ssize_t
vantage_get_archive(struct ws_archive *p, size_t nel, time_t after)
{
	ssize_t i, sz;
	struct vantage_dmp buf[nel];

	if (vantage_lock(fd) == -1) {
		goto error;
	}

	if ((sz = vantage_dmpaft(fd, buf, nel, after)) == -1) {
		syslog(LOG_ERR, "vantage_dmpaft: %m");
		goto error;
	}

	if (vantage_unlock(fd) == -1) {
		goto error;
	}

	/* Convert data */
	for (i = 0; i < sz; i++) {
		conv_ar_dmp(&p[i], &buf[i]);
	}

	return sz;

error:
	(void) vantage_unlock(fd);

	return -1;
}

int
vantage_set_artimer(long itmin, long next)
{
	errno = ENOTSUP;
	return -1;
}
