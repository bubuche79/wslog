#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/file.h>
#include <errno.h>
#include <syslog.h>

#include "defs/std.h"

#include "libws/util.h"
#include "libws/vantage/util.h"
#include "libws/vantage/vantage.h"

#include "driver/vantage.h"
#include "conf.h"

static int fd;				/* Device file */
static enum vantage_type wrd;		/* Console type */
static struct vantage_cfg cfg;		/* Console configuration */
static time_t current;			/* Last archive date */

static const double pow10d[] = { 1.0, 10.0, 100.0, 1000.0 };

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

	locked = 1;

	/* Wakeup console */
	if (vantage_wakeup(fd) == -1) {
		syslog(LOG_ERR, "vantage_wakeup: %m");
		goto error;
	}

	return 0;

error:
	if (locked) {
		(void) flock(fd, LOCK_UN);
	}
	return -1;
}

static int
vantage_unlock(int fd)
{
	if (flock(fd, LOCK_UN) == -1) {
		syslog(LOG_ERR, "flock (un): %m");
		goto error;
	}

	return 0;

error:
	return -1;
}

static time_t
vantage_ar_current(int fd)
{
	ssize_t sz;
	time_t after;
	struct vantage_dmp buf;

	after = time(NULL) - cfg.ar_period * 60;

	do {
		if ((sz = vantage_dmpaft(fd, &buf, 1, after)) == -1) {
			syslog(LOG_ERR, "vantage_dmpaft: %m");
			goto error;
		}

		if (sz > 0) {
			after = buf.tstamp;
		}
	} while (sz > 0);

	return after;

error:
	return -1;
}

static double
vantage_temp(int f, int scale)
{
	return (f / pow10d[scale] - 32) * 5 / 9;
}

static double
vantage_pressure(int p, int scale)
{
	return (p / pow10d[scale]) / 0.02952998751;
}

static double
vantage_rain(int ticks, int rain_cup)
{
	double r;

	if (rain_cup == 0) {
		r = 0.01 * 25.4;
	} else if (rain_cup == 1) {
		r = 0.2;
	} else {
		r = 0.1;
	}

	return r * ticks;
}

static double
vantage_speed(int s)
{
	return s * 1.609344 / 1000;
}

static void
vantage_ar_dmp(struct ws_archive *p, const struct vantage_dmp *d)
{
	p->data.time = d->tstamp;

	/* Handle dash values */
	if (d->temp != INT16_MAX) {
		p->data.wl_mask |= WF_TEMP;
		p->data.temp = vantage_temp(d->temp, 1);
	}
	if (d->humidity != UINT8_MAX) {
		p->data.wl_mask |= WF_HUMIDITY;
		p->data.humidity = d->humidity;
	}
	if (d->barometer != 0) {
		p->data.wl_mask |= WF_BAROMETER;
		p->data.barometer = vantage_pressure(d->barometer, 1);
	}

	if (d->in_temp != INT16_MAX) {
		p->data.wl_mask |= WF_TEMP_IN;
		p->data.temp_in = vantage_temp(d->in_temp, 1);
	}
	if (d->in_humidity != UINT8_MAX) {
		p->data.wl_mask |= WF_HUMIDITY_IN;
		p->data.humidity_in = d->in_humidity;
	}

	if (d->avg_wind_speed != UINT8_MAX) {
		p->data.wl_mask |= WF_WIND_SPEED;
		p->data.wind_speed = vantage_speed(d->avg_wind_speed);
	}
	if (d->main_wind_dir != UINT8_MAX) {
		p->data.wl_mask |= WF_WIND_DIR;
		p->data.wind_dir= d->main_wind_dir;
	}
	if (d->hi_wind_speed != UINT8_MAX) {
		p->data.wl_mask |= WF_WIND_GUST_SPEED;
		p->data.wind_gust_speed = vantage_speed(d->hi_wind_speed);
	}
	if (d->hi_wind_dir != UINT8_MAX) {
		p->data.wl_mask |= WF_WIND_GUST_DIR;
		p->data.wind_gust_dir= d->hi_wind_dir;
	}

	p->data.wl_mask |= WF_RAIN|WF_RAIN_RATE;
	p->data.rain = vantage_rain(d->rain, cfg.sb_rain_cup);
	p->data.rain_rate = vantage_rain(d->hi_rain_rate, cfg.sb_rain_cup);
}

int
vantage_init(void)
{
	char ftime[32];
	const char *tty = confp->driver.vantage.tty;

	if ((fd = vantage_open(tty)) == -1) {
		syslog(LOG_ERR, "vantage_open %s: %m", tty);
		goto error;
	}

	/* Read console configuration */
	if (vantage_lock(fd) == -1) {
		goto error;
	}

	if (vantage_wrd(fd, &wrd) == -1) {
		syslog(LOG_ERR, "vantage_wrd: %m");
		goto error;
	}
	if (vantage_ee_cfg(fd, &cfg) == -1) {
		syslog(LOG_ERR, "vantage_ee_cfg: %m");
		goto error;
	}

	/* Compute last archive timestamp */
	if ((current = vantage_ar_current(fd)) == -1) {
		goto error;
	}

	localftime_r(ftime, sizeof(ftime), &current, "%F %R");
	syslog(LOG_NOTICE, "last archive: %s", ftime);

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
		errno = ENOTSUP;

		// TODO
		it->it_interval.tv_sec = 2;
		it->it_interval.tv_nsec = 500*1000;
		it->it_value.tv_sec = 0;
		it->it_value.tv_nsec = 0;
	} else if (WS_ITIMER_ARCHIVE == type) {
		it->it_interval.tv_sec = cfg.ar_period * 60;
		it->it_interval.tv_nsec = 0;
		it->it_value.tv_sec = current + it->it_interval.tv_sec - time(NULL) + 10;
		it->it_value.tv_nsec = 0;
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
	errno = ENOTSUP;
	return -1;
}

ssize_t
vantage_get_archive(struct ws_archive *p, size_t nel)
{
	ssize_t i, sz;
	struct vantage_dmp buf[nel];

	if (vantage_lock(fd) == -1) {
		goto error;
	}

	if ((sz = vantage_dmpaft(fd, buf, nel, current)) == -1) {
		syslog(LOG_ERR, "vantage_dmpaft: %m");
		goto error;
	}

	/* Convert data */
	for (i = 0; i < sz; i++) {
		vantage_ar_dmp(&p[i], &buf[i]);
	}

	if (sz > 0) {
		current = buf[sz-1].tstamp;
	}

	if (vantage_unlock(fd) == -1) {
		return -1;
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
