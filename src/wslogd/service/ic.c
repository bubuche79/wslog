
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

	double temp;
	double barometer;
	int humidity;
	double dew_point;
	double wind_10m_dir;
	double wind_10m_speed;
	double hi_wind_10m_speed;
	double rain_rate;
	double hi_rain_rate_1h;
	double rain_1h;
	double rain_day;
	double rain_year;

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
dwrite(int fd, const char *fmt, ...)
{
	double v;
	va_list args;

	va_start(args, fmt);
	vdprintf(fd, fmt, args);
	va_end(args);
}

static void
ic_reset(struct ic *p)
{
	memset(p, 0, sizeof(p));

	aggr_init_avgdeg(&p->aggr.wind_10m_dir);
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

	gmftime(date, sizeof(date), &p->time, "%d/%m/%y");
	gmftime(time_utc, sizeof(time_utc), &p->time, "%H:%M");

	ret = dprintf(fd, "# INFORMATIONS\n"
			"id_station=%s\n"
			"type=txt\n"
			"version=%s-%s\n"
			"date_releve=%s\n"
			"heure_releve_utc=%s\n",
			confp->stat_ic.station,
			PACKAGE_NAME, PACKAGE_VERSION,
			date, time_utc);

	dwrite(fd, "temperature=%.1f\n", p->temp);
	dwrite(fd, "pression=%.1f\n", p->barometer);
	dwrite(fd, "humidite=%hhu\n", p->humidity);
	dwrite(fd, "point_de_rosee=%.1f\n", p->dew_point);
	if (p->wind_10m_speed > 0) {
		dwrite(fd, "vent_dir_moy=%d\n", (int) p->wind_10m_dir);
	} else {
		dwrite(fd, "vent_dir_moy=\n");
	}
	dwrite(fd, "vent_moyen=%.1f\n", p->wind_10m_speed);
	dwrite(fd, "vent_rafales=%.1f\n", p->hi_wind_10m_speed);
	dwrite(fd, "pluie_intensite=%.1f\n", p->rain_rate);
//	dwrite(fd, "pluie_intensite_maxi_1h=%.1f\n", p->hi_rain_rate_1h);
	dprintf(fd, "# PARAMETRES TEMPS PASSE\n");
	dprintf(fd, "pluie_cumul_1h=%.1f\n", p->rain_1h);
	dprintf(fd, "pluie_cumul=%.1f\n", p->rain_day);
	dprintf(fd, "pluie_cumul_heure_utc=%s\n", time_utc);
//	dprintf(fd, "pluie_cumul_annee=%.1f\n", p->rain_year);
//	."pluie_intensite_maxi=$max_rainRate_today\n"
//	."pluie_intensite_maxi_heure_utc=$max_rainRate_todayTime\n"
//	"tn_heure_utc=$min_temp_todayTime\n"
//	"tn_deg_c=$min_temp_today\n"
//	"tx_heure_utc=$max_temp_todayTime\n",
//	"tx_deg_c=$max_temp_today\n"
//	dprintf(fd, "# PARAMETRES TEMPS PASSE\n");
//	."radiations_solaires_wlk=$solar\n"
//	."uv_wlk=$uv\n";

	lseek(fd, 0, SEEK_SET);

	return ret;
}

static int
ic_put(const struct ic *p)
{
	int ret;
	CURL *curl = NULL;

	curl = curl_easy_init();
	if (curl) {
		char url[128];
		CURLcode code;

		ic_write(datfd, p);

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

	p->rain_rate = rt->rain_rate;
	p->rain_1h = rt->rain_1h;
	p->rain_day = rt->rain_day;

	if (rt->wind_speed > 0) {
		aggr_add(&p->aggr.wind_10m_dir, rt->wind_dir);
	}

	return 0;
}

int
ic_sig_ar(const struct ws_archive *ar)
{
	struct ic *curr = &dat[datidx];

	curr->time = ar->time;

	curr->temp = ar->temp;
	curr->barometer = ar->barometer;
	curr->humidity = ar->humidity;
	curr->dew_point = ws_dewpoint(ar->temp, ar->humidity);
	curr->wind_10m_dir = ar->avg_wind_dir * 22.5;
	curr->wind_10m_speed = ar->avg_wind_speed * 3.6;
	curr->hi_wind_10m_speed = ar->hi_wind_speed * 3.6;

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
