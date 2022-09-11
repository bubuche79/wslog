#ifndef _SERVICE_ARCHIVE_H
#define _SERVICE_ARCHIVE_H

#include <time.h>

#include "dataset.h"

#ifdef __cplusplus
extern "C" {
#endif

int archive_init(struct itimerspec *it);
int archive_destroy(void);

int archive_sig_timer(struct ws_archive *ar);

#ifdef __cplusplus
}
#endif

#endif /* _SERVICE_ARCHIVE_H */
