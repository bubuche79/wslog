#ifndef _CURL_H
#define _CURL_H

#include <curl/curl.h>

#ifdef __cplusplus
extern "C" {
#endif

void curl_log(const char *fn, CURLcode code);

CURLcode curl_easy_auth(CURL *h, const char *username, const char *pwd);
CURLcode curl_easy_upload(CURL *h, const char *url, int fd);

#ifdef __cplusplus
}
#endif

#endif /* _CURL_H */
