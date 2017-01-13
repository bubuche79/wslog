#ifndef _SQLITE_H
#define _SQLITE_H

/*
 * SQLite 3 support.
 */

#ifdef __cplusplus
extern "C" {
#endif

int sqlite_init(void);
int sqlite_write(void);
int sqlite_destroy(void);

#ifdef __cplusplus
}
#endif

#endif /* _SQLITE_H */
