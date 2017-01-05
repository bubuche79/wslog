#ifndef _WSLOGD_H
#define _WSLOGD_H

#include "conf.h"

#ifdef __cplusplus
extern "C" {
#endif

#define S_I644 (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)

const struct ws_conf *confp;

#ifdef __cplusplus
}
#endif

#endif /* _WSLOGD_H */
