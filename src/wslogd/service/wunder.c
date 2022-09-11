#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

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
#include "wunder.h"

#define URL_LEN		512
#define STATIC_URL 		"weatherstation.wunderground.com/weatherstation/updateweatherstation.php"

static long freq;

struct ws_http
{
	char *buf;
	size_t len;			/* Used size */
};

struct ws_wunder
{
	const char *param;
	int (*get) (const struct ws_loop *, double *);
	double (*conv) (double);
};

static const struct ws_wunder params[] =
{
	{ "winddir", ws_loop_wind_dir },
	{ "windspeedmph", ws_loop_wind_speed, ws_mph },
	{ "windgustdir_10m", ws_loop_hi_wind_10m_dir },
	{ "windgustmph_10m", ws_loop_hi_wind_10m_speed, ws_mph },
	{ "humidity", ws_loop_humidity },
	{ "dewptf", ws_loop_dew_point, ws_fahrenheit },
	{ "tempf", ws_loop_temp, ws_fahrenheit },
	{ "rainin", ws_loop_rain_1h, ws_in },
	{ "dailyrainin", ws_loop_rain_day, ws_in },
	{ "baromin", ws_loop_barometer, ws_inhg },
	{ "solarradiation", ws_loop_solar_rad, ws_inhg },
	{ "UV", ws_loop_uv },
	{ "indoortempf", ws_loop_in_temp, ws_fahrenheit },
	{ "indoorhumidity", ws_loop_in_humidity }
};

static size_t params_nel = array_size(params);

static void
http_init(struct ws_http *s)
{
	s->len = 0;
	s->buf = NULL;
}

static void
http_cleanup(struct ws_http *s)
{
	if (s != NULL) {
		free(s->buf);
	}
}

static size_t
http_write(char *ptr, size_t size, size_t nmemb, struct ws_http *s)
{
	size_t sz = size * nmemb;
	size_t new_len = s->len + sz;

	if (s->len < new_len) {
		s->len = new_len;
		s->buf = realloc(s->buf, s->len);

		if (s->buf == NULL) {
			syslog(LOG_ERR, "realloc: %m");
			return 0;
		}
	}

	memcpy(s->buf, ptr, sz);

	return sz;
}

static int
wunder_url(char *str, size_t len, CURL *h, const struct ws_loop *p)
{
	int ret;
	char ftime[22];			/* Date UTC */
	size_t i;

	/* Convert date */
	gmftime(ftime, sizeof(ftime), &p->time.tv_sec, "%F %T");

	/* URL encode parameters */
	char *dateutc = curl_easy_escape(h, ftime, 0);
	char *password = curl_easy_escape(h, confp->wunder.password, 0);

	/* Compute GET request */
	ret = snprintf(str, len,
			"%s://" STATIC_URL "?%s=%s&%s=%s&%s=%s&%s=%s.%.3ld",
			confp->wunder.https ? "https" : "http",
			"action", "updateraw",
			"ID", confp->wunder.station,
			"PASSWORD", password,
			"dateutc", dateutc, (p->time.tv_nsec / 1000000));
	if (ret == -1) {
		syslog(LOG_ERR, "snprintf: %m");
		goto error;
	} else if (len <= (size_t) ret) {
		syslog(LOG_ERR, "snprintf: Buffer overflow (%d bytes required)", ret);
		goto error;
	}

	str += ret;
	len -= ret;

	curl_free(dateutc);
	curl_free(password);

	for (i = 0; i < params_nel; i++) {
		double value;

		if (params[i].get(p, &value) == 0) {
			int ret;

			if (params[i].conv != NULL) {
				value = params[i].conv(value);
			}

			/* Add parameter */
			ret = snprintf(str, len, "&%s=%f", params[i].param, value);
			if (ret == -1) {
				syslog(LOG_ERR, "snprintf: %m");
				goto error;
			} else if (len <= (size_t) ret) {
				syslog(LOG_ERR, "snprintf: Buffer overflow (%d bytes required)", ret);
				goto error;
			}

			str += ret;
			len -= ret;
		}
	}

	return 0;

error:
	return -1;
}

/**
 * See http://wiki.wunderground.com/index.php/PWS_-_Upload_Protocol
 */
static int
wunder_perform(const struct ws_loop *p)
{
	int ret;
	CURL *curl = NULL;
	struct ws_http iobuf;
 
	http_init(&iobuf);

	curl = curl_easy_init();
	if (curl) {
		CURLcode code;
		char url[URL_LEN];

		if (wunder_url(url, sizeof(url), curl, p) == -1) {
			goto error;
		}

#ifdef DEBUG
		syslog(LOG_DEBUG, "Wunderground: %s", url);
#endif

		/* Set request option */
		code = curl_easy_setopt(curl, CURLOPT_URL, url);
		if (code != CURLE_OK) {
			curl_log("curl_easy_setopt", code);
			goto error;
		}
		code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, http_write);
		if (code != CURLE_OK) {
			curl_log("curl_easy_setopt", code);
			goto error;
		}
		code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &iobuf);
		if (code != CURLE_OK) {
			curl_log("curl_easy_setopt", code);
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

	/* Check response */
	ret = dry_run || (iobuf.buf && !strncmp("success\n", iobuf.buf, iobuf.len)) ? 0 : -1;
	if (ret == -1) {
		syslog(LOG_ERR, "Wunderground response: %*s", (int) iobuf.len, iobuf.buf);
	}

	http_cleanup(&iobuf);

	return ret;

error:
	if (curl) {
		curl_easy_cleanup(curl);
	}
	http_cleanup(&iobuf);

	return -1;
}

int
wunder_init(int *flags, struct itimerspec *it)
{
	CURLcode code;

	if (!confp->wunder.station || !confp->wunder.password) {
		syslog(LOG_ERR, "Wunderground: station or password not set");
		goto error;
	}

	/* Initialize */
	freq = confp->wunder.freq;
	if (freq == 0) {
		freq = 600;
	}

	itimer_setdelay(it, freq, 0);

	return 0;

error:
	return -1;
}

int
wunder_sig_timer(void)
{
	struct ws_loop arbuf;
	const struct ws_loop *p;

	/* Peek last archive element */
	if (board_lock() == -1) {
		syslog(LOG_CRIT, "board_lock: %m");
		goto error;
	}

	p = board_peek(0);

	if (p != NULL) {
		memcpy(&arbuf, p, sizeof(*p));
	}

	if (board_unlock() == -1) {
		syslog(LOG_CRIT, "board_unlock: %m");
		goto error;
	}

	/* Process archive element */
	if (p != NULL) {
		if (wunder_perform(&arbuf) == -1) {
			syslog(LOG_ERR, "Wunderground service error");

			/* Continue, not a fatal error */
		}
	}

	return 0;

error:
	return -1;
}

int
wunder_destroy(void)
{
	return 0;
}
