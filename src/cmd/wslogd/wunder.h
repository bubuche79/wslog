#ifndef _WUNDER_H
#define _WUNDER_H

/**
 * Weather underground support.
 */

#ifdef __cplusplus
extern "C" {
#endif

int wunder_init(void);
int wunder_update(void);
int wunder_destroy(void);

#ifdef __cplusplus
}
#endif

#endif	/* _WUNDER_H */
