#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <syslog.h>
#include <stdint.h>
#include <unistd.h>
#include <curl/curl.h>

static
ssize_t fdread(void *ptr, size_t size, size_t nmemb, intptr_t fd)
{
	return read(fd, ptr, size * nmemb);
}

void
curl_log(const char *fn, CURLcode code)
{
	syslog(LOG_ERR, "%s: %s (%d)", fn, curl_easy_strerror(code), code);
}

CURLcode
curl_easy_auth(CURL *h, const char *username, const char *passwd)
{
	CURLcode code;

	code = curl_easy_setopt(h, CURLOPT_USERNAME, username);
	if (code != CURLE_OK) {
		goto error;
	}

	code = curl_easy_setopt(h, CURLOPT_PASSWORD, passwd);
	if (code != CURLE_OK) {
		goto error;
	}

	return CURLE_OK;

error:
	return code;
}

CURLcode
curl_easy_upload(CURL *h, const char *url, int fd)
{
	CURLcode code;

	code = curl_easy_setopt(h, CURLOPT_URL, url);
	if (code != CURLE_OK) {
		goto error;
	}

	code = curl_easy_setopt(h, CURLOPT_UPLOAD, 1L);
	if (code != CURLE_OK) {
		goto error;
	}

	code = curl_easy_setopt(h, CURLOPT_READDATA, (intptr_t) fd);
	if (code != CURLE_OK) {
		goto error;
	}
	code = curl_easy_setopt(h, CURLOPT_READFUNCTION, fdread);
	if (code != CURLE_OK) {
		goto error;
	}

	return CURLE_OK;

error:
	return code;
}
