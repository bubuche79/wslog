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

#include "libws/util.h"
#include "libws/log.h"

#include "conf.h"
#include "board.h"
#include "wslogd.h"
#include "wunder.h"

#define URL "weatherstation.wunderground.com/weatherstation/updateweatherstation.php"

#define try_add(fn, str, len, p, isset, value) \
	do { \
		int _ret; \
		_ret = fn(str, len, (p), (isset), (value)); \
		if (_ret == -1) { \
			goto error; \
		} \
		str += _ret; \
		len -= _ret; \
	} while (0)

#define try_add_int(str, len, p, isset, value) \
	try_add(url_add_int, str, len, (p), (isset), (value))

#define try_add_float(str, len, p, isset, value) \
	try_add(url_add_float, str, len, (p), (isset), (value))

struct html {
	char *buf;
	size_t len;							/* used size */
};

struct ws_wunder {
	const char *param;
	int (*get) (const struct ws_loop *, double *);
	double (*conv) (double);
};

struct ws_wunder arr[] = {
	{ "baromin", ws_get_barometer, ws_inhg },
	{ "tempf", ws_get_temp, ws_fahrenheit },
	{ "humidity", ws_get_humidity, NULL },
	{ "windspeedmph", ws_get_wind_speed, ws_mph },
	{ "winddir", ws_get_wind_dir, NULL },
	{ "windgustmph", ws_get_wind_gust_speed, ws_mph },
	{ "windgustdir", ws_get_wind_gust_dir, NULL },
#if 0
	{ "rainin", ws_get_rain_1h, ws_in },
	{ "dailyrainin", ws_get_rain_24h, ws_in },
#endif
	{ "dewptf", ws_get_dew_point, ws_fahrenheit },
	{ "indoortempf", ws_get_temp_in, ws_fahrenheit },
	{ "indoorhumidity", ws_get_humidity_in, NULL },
};

static void
html_init(struct html* s)
{
	s->len = 0;
	s->buf = NULL;
}

static void
html_cleanup(struct html *s)
{
	if (s != NULL) {
		free(s->buf);
	}
}

static size_t
html_write(char *ptr, size_t size, size_t nmemb, struct html *s)
{
	size_t sz = size * nmemb;
	size_t new_len = s->len + sz;

	if (s->len < new_len) {
		s->len = new_len;
		s->buf = realloc(s->buf, s->len);

		if (s->buf == NULL) {
			csyslog1(LOG_ERR, "realloc(): %m");
			return 0;
		}
	}

	memcpy(s->buf, ptr, sz);

	return sz;
}

#if __GNUC__ >= 4
__attribute__ ((format (printf, 4, 5)))
#endif
static int
url_add(char *str, size_t len, int isset, const char *fmt, ...)
{
	int ret;

	if (isset) {
		va_list ap;

		va_start(ap, fmt);
		ret = vsnprintf(str, len, fmt, ap);
		va_end(ap);

		if (ret == -1) {
			csyslog1(LOG_ERR, "snprintf(): %m");
			goto error;
		} else if (len <= (size_t) ret) {
			syslog(LOG_ERR, "snprintf(): Buffer overflow (%d bytes required)", ret);
			goto error;
		}
	} else {
		ret = 0;
	}

	return ret;

error:
	return -1;
}

static int
url_add_int(char *str, size_t len, const char *p, int isset, int value)
{
	return url_add(str, len, isset, "&%s=%d", p, value);
}

static int
url_add_float(char *str, size_t len, const char *p, int isset, float value)
{
	return url_add(str, len, isset, "&%s=%f", p, value);
}

static void
curl_log(const char *fn, CURLcode code) {
	syslog(LOG_ERR, "%s(): %s (%d)", fn, curl_easy_strerror(code), code);
}

static int
wunder_url(char *str, size_t len, CURL *h, const struct ws_loop *p)
{
	int ret;
	char ctime[22];					/* date utc */

	/* Convert date */
	gmftime(ctime, sizeof(ctime), &p->time.tv_sec, "%F %T");

	/* URL encode parameters */
	char *dateutc = curl_easy_escape(h, ctime, 0);
	char *password = curl_easy_escape(h, confp->wunder.password, 0);

	/* Compute GET request */
	ret = snprintf(str, len,
			"%s://" URL "?%s=%s&%s=%s&%s=%s&%s=%s.%ld",
			confp->wunder.https ? "https" : "http",
			"action", "updateraw",
			"ID", confp->wunder.station,
			"PASSWORD", password,
			"dateutc", dateutc, p->time.tv_nsec);
	if (ret == -1) {
		csyslog1(LOG_ERR, "snprintf(): %m");
		goto error;
	} else if (len <= (size_t) ret) {
		syslog(LOG_ERR, "snprintf(): Buffer overflow (%d bytes required)", ret);
		goto error;
	}
	str += ret;
	len -= ret;

	curl_free(dateutc);
	curl_free(password);

	try_add_float(str, len, "baromin", ws_isset(p, WF_BAROMETER), ws_inhg(p->barometer));
	try_add_float(str, len, "tempf", ws_isset(p, WF_TEMP), ws_fahrenheit(p->temp));
	try_add_int(str, len, "humidity", ws_isset(p, WF_HUMIDITY), p->humidity);
	try_add_float(str, len, "windspeedmph",ws_isset(p, WF_WIND), ws_mph(p->wind_speed));
	try_add_int(str, len, "winddir", ws_isset(p, WF_WIND), p->wind_dir);
	try_add_float(str, len, "windgustmph", ws_isset(p, WF_WIND_GUST), ws_mph(p->wind_gust_speed));
	try_add_int(str, len, "windgustdir", ws_isset(p, WF_WIND_GUST), p->wind_gust_dir);
#if 0
	try_add_float(str, len, "rainin", ws_isset(p, WF_RAIN_RATE), ws_in(p->rain_1h));
	try_add_float(str, len, "dailyrainin", ws_isset(p, WF_SAMPLE_RAIN), ws_in(p->rain_24h));
#endif
	try_add_float(str, len, "dewptf", ws_isset(p, WF_DEW_POINT), ws_fahrenheit(p->dew_point));
	try_add_float(str, len, "indoortempf", ws_isset(p, WF_TEMP_IN), ws_fahrenheit(p->temp_in));
	try_add_int(str, len, "indoorhumidity", ws_isset(p, WF_HUMIDITY), p->humidity_in);

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
	struct html html;
 
	html_init(&html);

	curl = curl_easy_init();
	if (curl) {
		CURLcode code;
		char url[512];

		if (wunder_url(url, sizeof(url), curl, &p->data) == -1) {
			goto error;
		}

		/* Set request option */
		code = curl_easy_setopt(curl, CURLOPT_URL, url);
		if (code != CURLE_OK) {
			curl_log("curl_easy_setopt", code);
			goto error;
		}
		code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, html_write);
		if (code != CURLE_OK) {
			curl_log("curl_easy_setopt", code);
			goto error;
		}
		code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &html);
		if (code != CURLE_OK) {
			curl_log("curl_easy_setopt", code);
			goto error;
		}

#ifdef DEBUG
		printf("WUNDER: %s\n", url);
#endif

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
		syslog(LOG_ERR, "curl_easy_init() failed\n");
		goto error;
	}

	/* Check response */
	ret = dry_run || (html.buf && !strncmp("success\n", html.buf, html.len)) ? 0 : -1;
	if (ret == -1) {
		syslog(LOG_ERR, "wunderground response: %*s", (int) html.len, html.buf);
	}

	html_cleanup(&html);

	return ret;

error:
	if (curl) {
		curl_easy_cleanup(curl);
	}
	html_cleanup(&html);

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
		csyslog1(LOG_CRIT, "board_lock(): %m");
		goto error;
	}

	p = board_peek_ar(0);

	if (p != NULL) {
		memcpy(&arbuf, p, sizeof(*p));
	}

	if (board_unlock() == -1) {
		csyslog1(LOG_CRIT, "board_unlock(): %m");
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
