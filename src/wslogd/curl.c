#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <syslog.h>
#include <curl/curl.h>

void
curl_log(const char *fn, CURLcode code)
{
	syslog(LOG_ERR, "%s: %s (%d)", fn, curl_easy_strerror(code), code);
}
