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

#include "defs/std.h"
#include "libws/util.h"

#include "conf.h"
#include "board.h"
#include "wslogd.h"
#include "wunder.h"

#define URL "weatherstation.wunderground.com/weatherstation/updateweatherstation.php"

struct ws_http
{
	char *buf;
	size_t len;			/* used size */
};

struct ws_wunder
{
	const char *param;
	int (*get) (const struct ws_archive *, double *);
	double (*conv) (double);
};

static struct ws_wunder params[] =
{
	{ "baromin", ws_get_barometer, ws_inhg },
	{ "tempf", ws_get_temp, ws_fahrenheit },
	{ "humidity", ws_get_humidity, NULL },
	{ "windspeedmph", ws_get_wind_speed, ws_mph },
	{ "winddir", ws_get_wind_dir, NULL },
	{ "windgustmph", ws_get_hi_wind_speed, ws_mph },
	{ "windgustdir", ws_get_hi_wind_dir, NULL },
	{ "dewptf", ws_get_dew_point, ws_fahrenheit },
	{ "indoortempf", ws_get_in_temp, ws_fahrenheit },
	{ "indoorhumidity", ws_get_in_humidity, NULL }
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

static void
curl_log(const char *fn, CURLcode code)
{
	syslog(LOG_ERR, "%s: %s (%d)", fn, curl_easy_strerror(code), code);
}

static int
wunder_url(char *str, size_t len, CURL *h, const struct ws_archive *p)
{
	int ret;
	char ftime[22];			/* date utc */
	size_t i;

	/* Convert date */
	gmftime(ftime, sizeof(ftime), &p->time, "%F %T");

	/* URL encode parameters */
	char *dateutc = curl_easy_escape(h, ftime, 0);
	char *password = curl_easy_escape(h, confp->wunder.password, 0);

	/* Compute GET request */
	ret = snprintf(str, len,
			"%s://" URL "?%s=%s&%s=%s&%s=%s&%s=%s.%ld",
			confp->wunder.https ? "https" : "http",
			"action", "updateraw",
			"ID", confp->wunder.station,
			"PASSWORD", password,
			"dateutc", dateutc, p->time);
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
wunder_perform(const struct ws_archive *p)
{
	int ret;
	CURL *curl = NULL;
	struct ws_http iobuf;
 
	http_init(&iobuf);

	curl = curl_easy_init();
	if (curl) {
		CURLcode code;
		char url[512];

		if (wunder_url(url, sizeof(url), curl, p) == -1) {
			goto error;
		}

#ifdef DEBUG
		syslog(LOG_NOTICE, "Wunder: %s", url);
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
		syslog(LOG_ERR, "wunderground response: %*s", (int) iobuf.len, iobuf.buf);
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
wunder_init(void)
{
	CURLcode code;

	code = curl_global_init(CURL_GLOBAL_DEFAULT);
	if (code != CURLE_OK) {
		curl_log("curl_global_init", code);
		goto error;
	}

	return 0;

error:
	return -1;
}

int
wunder_update(void)
{
	struct ws_archive arbuf;
	const struct ws_archive *p;

	/* Peek last archive element */
	if (board_lock() == -1) {
		syslog(LOG_CRIT, "board_lock: %m");
		goto error;
	}

	p = board_peek_ar(0);

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
			syslog(LOG_ERR, "wunder service error");

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
	curl_global_cleanup();

	return 0;
}
