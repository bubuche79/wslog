#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <syslog.h>

#include "libws/util.h"

#include "conf.h"
#include "board.h"
#include "wslogd.h"
#include "wunder.h"

#define URL "https://weatherstation.wunderground.com/weatherstation/updateweatherstation.php"

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
			syslog(LOG_ERR, "realloc(): %m");
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
			syslog(LOG_ERR, "snprintf(): %m");
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
			URL "?%s=%s&%s=%s&%s=%s&%s=%s.%ld",
			"action", "updateraw",
			"ID", confp->wunder.station,
			"PASSWORD", password,
			"dateutc", dateutc, p->time.tv_nsec);
	if (ret == -1) {
		syslog(LOG_ERR, "snprintf(): %m");
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
	try_add_float(str, len, "windgustmph", ws_isset(p, WF_WIND_GUST), ws_mph(p->wind_gust));
	try_add_int(str, len, "windgustdir", ws_isset(p, WF_WIND_GUST), p->wind_gust_dir);
	try_add_float(str, len, "rainin", ws_isset(p, WF_RAIN_RATE), ws_inch(p->rain_1h));
	try_add_float(str, len, "dailyrainin", ws_isset(p, WF_SAMPLE_RAIN), ws_inch(p->rain_24h));
	try_add_float(str, len, "dewptf", ws_isset(p, WF_DEW_POINT), ws_fahrenheit(p->dew_point));
	try_add_float(str, len, "indoortempf", ws_isset(p, WF_TEMP_IN), ws_fahrenheit(p->temp_in));
	try_add_int(str, len, "indoorhumidity", ws_isset(p, WF_HUMIDITY), p->humidity_in);

	return 0;

error:
	return -1;
}

int
wunder_init(void)
{
	CURLcode code;

	code = curl_global_init(CURL_GLOBAL_DEFAULT);
	if (code != CURLE_OK) {
		syslog(LOG_ERR, "curl_global_init(): %s", curl_easy_strerror(code));
		goto error;
	}

	return 0;

error:
	return -1;
}

/**
 * See http://wiki.wunderground.com/index.php/PWS_-_Upload_Protocol
 */
int
wunder_write(const struct ws_archive *p)
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
			syslog(LOG_ERR, "curl_easy_setopt(): %s", curl_easy_strerror(code));
			goto error;
		}
		code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, html_write);
		if (code != CURLE_OK) {
			syslog(LOG_ERR, "curl_easy_setopt(): %s", curl_easy_strerror(code));
			goto error;
		}
		code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &html);
		if (code != CURLE_OK) {
			syslog(LOG_ERR, "curl_easy_setopt(): %s", curl_easy_strerror(code));
			goto error;
		}

		/* Perform request */
		code = curl_easy_perform(curl);
		if (code != CURLE_OK) {
			syslog(LOG_ERR, "curl_easy_perform(): %s", curl_easy_strerror(code));
			goto error;
		}

		/* Cleanup */
		curl_easy_cleanup(curl);
		curl = NULL;
	} else {
		syslog(LOG_ERR, "curl_easy_init() failed\n");
		goto error;
	}

	/* Check response */
	ret = (html.buf && !strcmp("success\n", html.buf)) ? 0 : -1;
	if (ret == -1) {
		syslog(LOG_ERR, "wunderground response: %s", html.buf);
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
wunder_destroy(void)
{
	curl_global_cleanup();

	return 0;
}