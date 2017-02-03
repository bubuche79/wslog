#ifndef _DB_SQLITE_H
#define _DB_SQLITE_H

#include "dataset.h"

/*
 * SQLite 3 support.
 */

#ifdef __cplusplus
extern "C" {
#endif

int sqlite_init(void);
ssize_t sqlite_insert(const struct ws_archive *p, size_t nel);
ssize_t sqlite_select_last(struct ws_archive *p, size_t nel);
int sqlite_destroy(void);

#ifdef __cplusplus
}
#endif

#endif /* _DB_SQLITE_H */
