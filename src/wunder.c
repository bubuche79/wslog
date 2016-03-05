#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

#include "util.h"
#include "wunder.h"

#define URL			"http://weatherstation.wunderground.com/weatherstation/updateweatherstation.php"
#define ID			"IMIDIPYR166"
#define PASSWORD	"m7qg8QR5kXbFU2G5pcGbBqK9"

static size_t
gmftime(char *s, size_t max, const time_t *timep, const char *fmt)
{
	struct tm tm;

	gmtime_r(timep, &tm);
	return strftime(s, max, fmt, &tm);
}

/**
 * http://wiki.wunderground.com/index.php/PWS_-_Upload_Protocol
 */
int
ws_wunder_upload(const struct ws_wunder *w)
{
	CURL *curl;
	CURLcode res;
 
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

		} else if (sizeof(url) <= (size_t) sz) {
			fprintf(stderr, "Buffer overflow for URL (%d bytes required)", sz);
			return -1;
		}

		curl_free(dateutc);
		curl_free(password);

		printf("%s\n", url);
		curl_easy_setopt(curl, CURLOPT_URL, url);

		/* Perform the request, res will get the return code */ 
		res = curl_easy_perform(curl);

		if (res != CURLE_OK) {
			goto curl_error;
		}
 
		/* Cleanup */
		curl_easy_cleanup(curl);
	} else {
		fprintf(stderr, "libcurl init error");
		return -1;
	}

	curl_global_cleanup();

	return 0;

curl_error:
	fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

	curl_easy_cleanup(curl);
	curl_global_cleanup();

	return -1;
}
