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

#include "conf.h"
#include "curl.h"
#include "board.h"
#include "wslogd.h"
#include "service/util.h"
#include "stat_ic.h"

#define STATIC_URL 		"ftp.infoclimat.fr"
#define STATIC_VERSION		PACKAGE_VERSION

static long freq;
static int datfd;		/* Output file */

struct ic_dat {
	time_t next;
	struct ws_loop rt;

	struct aggr rain_1h;
	struct aggr wind_speed_10m;
};

static struct ic_dat dat;

static void
dwrite(int fd, const char *fmt, const struct ws_archive *p, int (*fn)(const struct ws_archive *, double *))
{
	double v;

	if (fn(p, &v) == 0) {
		dprintf(fd, fmt, v);
	}
}

/**
 * See https://github.com/AssociationInfoclimat/
 */
static ssize_t
static_write(int fd, const struct ws_loop *p)
{
	ssize_t ret;
	char date[20];
	char time_utc[20];

	lseek(fd, 0, SEEK_SET);

	gmftime(date, sizeof(date), &p->time, "%d/%m/%y");
	gmftime(time_utc, sizeof(time_utc), &p->time, "%H:%M");

	ret = dprintf(fd, "# INFORMATIONS\n"
			"id_station=%s\n"
			"type=txt\n"
			"version=%s\n"
			"date_releve=%s\n"
			"heure_releve_utc=%s\n",
			confp->stat_ic.station,
			STATIC_VERSION,
			date, time_utc);

	dwrite(fd, "temperature=%.1f\n", p, p->temp);
	dwrite(fd, "pression=%1.f\n", p, p->barometer);
	dwrite(fd, "humidite=%hhu\n", p, p->humidity);
	dwrite(fd, "point_de_rosee=%1.f\n", p, p->dew_point);
	dwrite(fd, "vent_dir_moy=%0.f\n", p, p->avg_wind_dir);
	dwrite(fd, "vent_moyen%1.f\n", p, p->avg_wind_speed);
	dwrite(fd, "vent_rafales=%.1f\n", p, p->hi_wind_speed);
	dwrite(fd, "pluie_intensite=%.1f\n", p, p->rain_rate);
	dwrite(fd, "pluie_intensite_maxi_1h=%.1f\n", p->hi_rain_rate);
//	dprintf(fd, "# PARAMETRES TEMPS PASSE\n");
//	dprintf(fd, "pluie_cumul_1h=%.1f\n", p, p->rain_1h);
//	dprintf(fd, "pluie_cumul=%.1f\n", p, p->rain_today);
//	dprintf(fd, "pluie_cumul_heure_utc=%1.f\n", p, p->rain_
//	"tn_deg_c=$min_temp_today\n"
//	"tn_heure_utc=$min_temp_todayTime\n"
//	"tx_deg_c=$max_temp_today\n"
//	"tx_heure_utc=$max_temp_todayTime\n",

	lseek(fd, 0, SEEK_SET);

	return ret;
}

static int
static_put(const struct ws_archive *p)
{
	int ret;
	CURL *curl = NULL;

	curl = curl_easy_init();
	if (curl) {
		char url[128];
		CURLcode code;

		static_write(datfd, p);

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
static_init(int *flags, struct itimerspec *it)
{
	char template[20] = "/tmp/wslogXXXXXX";

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

	return 0;

error:
	return -1;
}

int
static_destroy(void)
{
	(void) close(datfd);

	return 0;
}

int
static_sig_rt(const struct ws_loop *rt)
{
	if (rt->time <= dat.next) {
		memcpy(dat.rt, rt, sizeof(dat.rt));
	}

	return 0;
}

int
static_sig_ar(const struct ws_archive *ar)
{
	/* Process archive element */
	if (static_put(ar) == -1) {
		syslog(LOG_ERR, "StatIC service error");

		/* Continue, not a fatal error */
	}

	/* Next archive time */
	dat.next = ar->time + confp->archive.freq;

	return 0;

error:
	return -1;
}
