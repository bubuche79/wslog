#ifndef _CSV_H
#define _CSV_H

/*
 * CSV output.
 */

#ifdef __cplusplus
extern "C" {
#endif

int csv_init(void);
int csv_write(void);
int csv_destroy(void);

#ifdef __cplusplus
}
#endif

#endif /* _CSV_H */
