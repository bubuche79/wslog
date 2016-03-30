#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <errno.h>

#include "util.h"
#include "wunder.h"

#define URL			"http://weatherstation.wunderground.com/weatherstation/updateweatherstation.php"
#define ID			"IMIDIPYR166"
#define PASSWORD	"m7qg8QR5kXbFU2G5pcGbBqK9"

struct html {
	char *ptr;
	size_t len;							/* used size */
};

static size_t
gmftime(char *s, size_t max, const time_t *timep, const char *fmt)
{
	struct tm tm;

	gmtime_r(timep, &tm);
	return strftime(s, max, fmt, &tm);
}

static void
html_init(struct html* s)
{
	s->len = 0;
	s->ptr = NULL;
}

static void
html_cleanup(struct html *s)
{
	if (s != NULL) {
		free(s->ptr);
	}
}

static size_t
html_write(char *ptr, size_t size, size_t nmemb, struct html *s)
{
	size_t sz = size * nmemb;
	size_t new_len = s->len + sz;

	s->ptr = realloc(s->ptr, new_len + 1);
	if (s->ptr == NULL) {
		fprintf(stderr, "realloc() failed\n");
		exit(1);		// TODO
	}

	memcpy(s->ptr + s->len, ptr, sz);
	s->ptr[new_len] = '\0';
	s->len = new_len;

	return sz;
}

/**
 * http://wiki.wunderground.com/index.php/PWS_-_Upload_Protocol
 */
int
ws_wunder_upload(const struct ws_wunder *w)
{
	int rc;

	CURLcode res;
	CURL *curl = NULL;
	struct html p;						/* GET output data */
 
	html_init(&p);
	curl_global_init(CURL_GLOBAL_DEFAULT);

	curl = curl_easy_init();

	if (curl) {
		int sz;
		char url[512];					/* final url */
		char ctime[22];					/* date utc */

		/* Convert date */
		gmftime(ctime, sizeof(ctime), &w->time, "%F %T");

		/* URL encode parameters */
		char *dateutc = curl_easy_escape(curl, ctime, 0);
		char *password = curl_easy_escape(curl, PASSWORD, 0);

		/* Compute GET request */
		sz = snprintf(url, sizeof(url),
				"%s?%s=%s&%s=%s&%s=%s&%s=%s&%s=%d&%s=%f&%s=%d&%s=%f&%s=%f&%s=%f&%s=%f&%s=%f&%s=%d",
				URL,
				"action", "updateraw",
				"ID", ID,
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
			fprintf(stderr, "snprintf(): %s\n", strerror(errno));
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

	curl_global_cleanup();

	/* Check response */
	rc = strcmp("success\n", p.ptr);

	if (rc) {
		fprintf(stderr, "wunderground.com response:\n%s\n", p.ptr);
	}

	html_cleanup(&p);

	return rc ? -1 : 0;

curl_error:
	fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

error:
	if (curl != NULL) {
		curl_easy_cleanup(curl);
	}
	curl_global_cleanup();
	html_cleanup(&p);

	return -1;
}
