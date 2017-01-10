#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <errno.h>
#include <syslog.h>

#include "libws/util.h"

#include "conf.h"
#include "board.h"
#include "wslogd.h"
#include "wunder.h"

#define WUNDER_URL "http://weatherstation.wunderground.com/weatherstation/updateweatherstation.php"

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

	s->buf = realloc(s->buf, new_len + 1);
	if (s->buf == NULL) {
		fprintf(stderr, "realloc() failed\n");
		exit(1);		// TODO
	}

	memcpy(s->buf + s->len, ptr, sz);
	s->buf[new_len] = '\0';
	s->len = new_len;

	return sz;
}

int
wunder_init(void)
{
	curl_global_init(CURL_GLOBAL_DEFAULT);

	return 0;
}

/**
 * See
 */
int
wunder_update(void)
{
	int rc;

	CURLcode res;
	CURL *curl = NULL;
	struct html p;						/* GET output data */
 
	html_init(&p);

	curl = curl_easy_init();
	if (curl) {
		int sz;
		char url[512];					/* final url */
		char ctime[22];					/* date utc */
		const struct ws_ws23xx *w;

		w = board_last();

		/* Convert date */
		gmftime(ctime, sizeof(ctime), &w->time, "%F %T");

		/* URL encode parameters */
		char *dateutc = curl_easy_escape(curl, ctime, 0);
		char *password = curl_easy_escape(curl, confp->wunder.user_pwd, 0);

		/* Compute GET request */
		sz = snprintf(url, sizeof(url),
				"%s?%s=%s&%s=%s&%s=%s&%s=%s&%s=%d&%s=%f&%s=%d&%s=%f&%s=%f&%s=%f&%s=%f&%s=%f&%s=%d",
				WUNDER_URL,
				"action", "updateraw",
				"ID", confp->wunder.user_id,
				"PASSWORD", password,
				"dateutc", dateutc,
				"winddir", w->wind_dir,
				"windspeedmph", ws_mph(w->wind_speed),
				"humidity", w->humidity,
				"dewptf", ws_fahrenheit(w->dew_point),
				"tempf", ws_fahrenheit(w->temp),
				"rainin", ws_inch(w->rain),
				"dailyrainin", ws_inch(w->daily_rain),
				"indoortempf", ws_fahrenheit(w->temp_in),
				"indoorhumidity", w->humidity_in);
		if (sz == -1) {
			syslog(LOG_ERR, "snprintf(): %m");
			goto error;
		} else if (sizeof(url) <= (size_t) sz) {
			fprintf(stderr, "Buffer overflow for URL (%d bytes required)", sz);
			return -1;
		}

		curl_free(dateutc);
		curl_free(password);

		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, html_write);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &p);

		/* Perform the request, res will get the return code */ 
		res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			goto curl_error;
		}
 
		/* Cleanup */
		curl_easy_cleanup(curl);
		curl = NULL;
	} else {
		fprintf(stderr, "curl_easy_init() failed\n");
		goto error;
	}

	/* Check response */
	rc = strcmp("success\n", p.buf);

	if (rc) {
		fprintf(stderr, "wunderground.com response:\n%s\n", p.buf);
	}

	html_cleanup(&p);

	return rc ? -1 : 0;

curl_error:
	fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

error:
	if (curl != NULL) {
		curl_easy_cleanup(curl);
	}
	html_cleanup(&p);

	return -1;
}

int
wunder_destroy(void)
{
	curl_global_cleanup();

	return 0;
}
