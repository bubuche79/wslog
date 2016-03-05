#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

#include "util.h"
#include "wunder.h"

#define URL			"http://weatherstation.wunderground.com/weatherstation/updateweatherstation.php"
#define ID			"IMIDIPYR166"
#define PASSWORD	"m7qg8QR5kXbFU2G5pcGbBqK9"

static size_t
strfutc(char *s, size_t max, const time_t *timep, const char *fmt)
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
 
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();

	if (curl) {
		CURLcode res;

		char url[1024];					/* final url */
		char ctime[24];					/* date utc */

		/* Convert date */
		strfutc(ctime, sizeof(ctime), &w->time, "%F %T");

		/* URL encode parameters */
		char *dateutc = curl_easy_escape(curl, ctime, 0);
		char *password = curl_easy_escape(curl, PASSWORD, 0);

		/* Compute GET request */
		sprintf(url, "%s?action=%s&ID=%s&PASSWORD=%s&dateutc=%s&winddir=%d&windspeedmph=%.2f&humidity=%d&dewptf=%.2f&tempf=%.2f&rainin=%.2f&dailyrainin=%.2f&indoortempf=%.2f&indoorhumidity=%d",
				URL, "updateraw", ID,
				password,
				dateutc,
				w->wind_dir,
				ws_mph(w->wind_speed),
				w->humidity,
				ws_fahrenheit(w->dew_point),
				ws_fahrenheit(w->temp),
				ws_inch(w->rain),
				ws_inch(w->daily_rain),
				ws_fahrenheit(w->temp_in),
				w->humidity_in);

		curl_free(dateutc);
		curl_free(password);

		printf("%s\n", url);
		curl_easy_setopt(curl, CURLOPT_URL, url);

		/* Perform the request, res will get the return code */ 
//		res = curl_easy_perform(curl);
		/* Check for errors */ 
		if (res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		}
 
		/* Cleanup */
		curl_easy_cleanup(curl);		
	}

	curl_global_cleanup();

	return 0;
}

