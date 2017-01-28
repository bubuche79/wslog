#ifndef _SERVICE_ARCHIVE_H
#define _SERVICE_ARCHIVE_H

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

int archive_init(struct itimerspec *it);
int archive_main(void);
int archive_destroy(void);

#ifdef __cplusplus
}
#endif

#endif /* _SERVICE_ARCHIVE_H */
