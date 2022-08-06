#ifndef _CURL_H
#define _CURL_H

#include <curl/curl.h>

#ifdef __cplusplus
extern "C" {
#endif

void curl_log(const char *fn, CURLcode code);

#ifdef __cplusplus
}
#endif

#endif /* _CURL_H */
