
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <syslog.h>

#include "libws/defs.h"
#include "libws/util.h"
#include "libws/aggregate.h"

#include "conf.h"
#include "curl.h"
#include "board.h"
#include "wslogd.h"
#include "service/util.h"
#include "ic.h"

#define STATIC_URL 		"ftp.infoclimat.fr"

struct ic {
	time_t time;
	uint32_t ic_mask;

	double temp;
	double barometer;
	int humidity;
	double dew_point;
	int wind_10m_dir;
	double wind_10m_speed;
	double hi_wind_10m_speed;
	double rain_rate;
	double hi_rain_rate_1h;
	double rain_1h;
	double rain_day;
	double rain_year;

	uint16_t solar_rad;
	double uv_idx;

	/* Aggregate data */
	struct {
		struct aggr wind_10m_dir;
	} aggr;
};

static long freq;
static int datfd;		/* Output file */

static time_t next;
static int datidx;
static struct ic dat[2];

static void
ic_reset(struct ic *p)
{
	memset(p, 0, sizeof(p));

	aggr_init_avgdeg(&p->aggr.wind_10m_dir);
}

static void
ic_writev(int fd, int mask, int flag, const char *n, const char *fmt, ...)
{
	dprintf(fd, "%s=", n);

	if (WF_ISSET(mask, flag)) {
		va_list ap;

		va_start(ap, fmt);
		vdprintf(fd, fmt, ap);
		va_end(ap);
	}

	dprintf(fd, "\n");
}

/**
 * See https://github.com/AssociationInfoclimat/
 */
static ssize_t
ic_write(int fd, const struct ic *p)
{
	ssize_t ret;
	char date[20];
	char time_utc[20];

	lseek(fd, 0, SEEK_SET);
	ftruncate(fd, 0);

	gmftime(date, sizeof(date), &p->time, "%d/%m/%Y");
	gmftime(time_utc, sizeof(time_utc), &p->time, "%H:%M");

	ret = dprintf(fd, "id_station=%s\n"
			"type=txt\n"
			"version=%s-%s\n"
			"date_releve=%s\n"
			"heure_releve_utc=%s\n",
			confp->stat_ic.station,
			PACKAGE_NAME, PACKAGE_VERSION,
			date, time_utc);

	ic_writev(fd, p->ic_mask, WF_TEMP, "temperature", "%.1f", p->temp);
	ic_writev(fd, p->ic_mask, WF_BAROMETER, "pression", "%.1f", p->barometer);
	ic_writev(fd, p->ic_mask, WF_HUMIDITY, "humidite", "%hhu", p->humidity);
	ic_writev(fd, p->ic_mask, WF_DEW_POINT, "point_de_rosee", "%.1f", p->dew_point);
	ic_writev(fd, p->ic_mask, WF_WIND_DIR, "vent_dir_moy", "%d", p->wind_10m_dir);
	ic_writev(fd, p->ic_mask, WF_WIND_SPEED, "vent_moyen", "%.1f", p->wind_10m_speed);
	ic_writev(fd, p->ic_mask, WF_HI_WIND_SPEED, "vent_rafales", "%.1f", p->hi_wind_10m_speed);
	ic_writev(fd, p->ic_mask, WF_RAIN_RATE, "pluie_intensite", "%.1f", p->rain_rate);
	//	dwrite(fd, "pluie_intensite_maxi_1h=%.1f\n", p->hi_rain_rate_1h);

	ic_writev(fd, p->ic_mask, WF_RAIN_1H, "pluie_cumul_1h", "%.1f", p->rain_1h);
	ic_writev(fd, p->ic_mask, WF_RAIN_DAY, "pluie_cumul", "%.1f", p->rain_day);
	dprintf(fd, "pluie_cumul_heure_utc=%s\n", time_utc);

	ic_writev(fd, p->ic_mask, WF_SOLAR_RAD, "radiations_solaires_wlk", "%hu", p->solar_rad);
	ic_writev(fd, p->ic_mask, WF_UV_INDEX, "uv_wlk", "%.1f", p->uv_idx);

	lseek(fd, 0, SEEK_SET);

	return ret;
}

static int
ic_put(const struct ic *p)
{
	int ret;
	CURL *curl = NULL;

	ic_write(datfd, p);

	curl = curl_easy_init();

	if (curl) {
		char url[128];
		CURLcode code;

		snprintf(url, sizeof(url), "ftp://" STATIC_URL "/StatIC_%s.txt", confp->stat_ic.station);

		code = curl_easy_auth(curl, confp->stat_ic.username, confp->stat_ic.password);
		if (code != CURLE_OK) {
			curl_log("curl_easy_auth", code);
			goto error;
		}

		code = curl_easy_upload(curl, url, datfd);
		if (code != CURLE_OK) {
			curl_log("curl_easy_upload", code);
			goto error;
		}

		/* Perform request */
		if (!dry_run) {
			code = curl_easy_perform(curl);
			if (code != CURLE_OK) {
				curl_log("curl_easy_perform", code);
				goto error;
			}
		}

		/* Cleanup */
		curl_easy_cleanup(curl);
		curl = NULL;
	} else {
		syslog(LOG_ERR, "curl_easy_init: failure\n");
		goto error;
	}

	return 0;

error:
	if (curl) {
		curl_easy_cleanup(curl);
	}

	return -1;
}

int
ic_init(int *flags, struct itimerspec *it)
{
	char template[20] = "/tmp/wslogXXXXXX";

	/* Check parameters */
	if (!confp->stat_ic.station || !confp->stat_ic.username || !confp->stat_ic.password) {
		syslog(LOG_ERR, "StatIC: station, username or password not set");
		goto error;
	}

	/* Initialize */
	freq = confp->stat_ic.freq;
	if (freq == 0) {
		freq = 600;
	}

	if ((datfd = mkstemp(template)) == -1) {
		syslog(LOG_ERR, "mkstemp: %m");
		goto error;
	}

	itimer_setdelay(it, freq, 0);

	*flags = SRV_EVENT_RT | SRV_EVENT_AR;

	/* Internal data */
	datidx = 0;
	next = 0;

	ic_reset(&dat[0]);
	ic_reset(&dat[1]);

	return 0;

error:
	return -1;
}

int
ic_destroy(void)
{
	(void) close(datfd);

	return 0;
}

int
ic_sig_rt(const struct ws_loop *rt)
{
	struct ic *p;

	if (!next || rt->time.tv_sec < next) {
		p = &dat[datidx];
	} else {
		p = &dat[(datidx + 1) & 1];
	}

	if (WF_ISSET(rt->wl_mask, WF_RAIN_RATE)) {
		p->ic_mask |= WF_RAIN_RATE;
		p->rain_rate = rt->rain_rate;
	}
	if (WF_ISSET(rt->wl_mask, WF_RAIN_1H)) {
		p->ic_mask |= WF_RAIN_1H;
		p->rain_1h = rt->rain_1h;
	}
	if (WF_ISSET(rt->wl_mask, WF_RAIN_DAY)) {
		p->ic_mask |= WF_RAIN_DAY;
		p->rain_day = rt->rain_day;
	}

//	if (WF_ISSET(rt->wl_mask, WF_WIND_DIR)) {
//		p->ic_mask |= WF_10M_WIND_DIR;
//		aggr_add(&p->aggr.wind_10m_dir, rt->wind_dir);
//	}

	if (WF_ISSET(rt->wl_mask, WF_SOLAR_RAD)) {
		p->ic_mask |= WF_SOLAR_RAD;
		p->solar_rad = rt->solar_rad;
	}
	if (WF_ISSET(rt->wl_mask, WF_UV_INDEX)) {
		p->ic_mask |= WF_UV_INDEX;
		p->uv_idx = rt->uv_idx;
	}

	return 0;
}

int
ic_sig_ar(const struct ws_archive *ar)
{
	struct ic *curr = &dat[datidx];

	curr->time = ar->time;

	if (WF_ISSET(ar->wl_mask, WF_TEMP)) {
		curr->ic_mask |= WF_TEMP;
		curr->temp = ar->temp;
	}
	if (WF_ISSET(ar->wl_mask, WF_BAROMETER)) {
		curr->ic_mask |= WF_BAROMETER;
		curr->barometer = ar->barometer;
	}
	if (WF_ISSET(ar->wl_mask, WF_HUMIDITY)) {
		curr->ic_mask |= WF_HUMIDITY;
		curr->humidity = ar->humidity;
	}
	if (WF_ISSET(ar->wl_mask, WF_TEMP|WF_HUMIDITY)) {
		curr->ic_mask |= WF_DEW_POINT;
		curr->dew_point = ws_dewpoint(ar->temp, ar->humidity);
	}
	if (WF_ISSET(ar->wl_mask, WF_WIND_DIR)) {
		curr->ic_mask |= WF_WIND_DIR;
		curr->wind_10m_dir = ar->avg_wind_dir * 22.5;
	}
	if (WF_ISSET(ar->wl_mask, WF_WIND_SPEED)) {
		curr->ic_mask |= WF_WIND_SPEED;
		curr->wind_10m_speed = ar->avg_wind_speed * 3.6;
	}
	if (WF_ISSET(ar->wl_mask, WF_HI_WIND_SPEED)) {
		curr->ic_mask |= WF_HI_WIND_SPEED;
		curr->hi_wind_10m_speed = ar->hi_wind_speed * 3.6;
	}

	/* Aggregate */
//	curr->rain_year += ar->rain_fall;
//
//	if (curr->wind_10m_speed > 0) {
//		aggr_finish(&curr->aggr.wind_10m_dir, &curr->wind_10m_dir);
//	}

	/* Process archive element */
	if (ic_put(curr) == -1) {
		syslog(LOG_ERR, "StatIC service error");

		/* Continue, not a fatal error */
	}

	/* Next archive time */
	ic_reset(curr);

	datidx = (datidx + 1) & 1;
	next = ar->time + freq;

	return 0;

error:
	return -1;
}
