#ifndef _USYS_STD_H
#define _USYS_STD_H

/*
 * Contains some often-used functions, prototypes, etc.
 */

#ifdef __cplusplus
extern "C" {
#endif

#define array_size(x) (sizeof(x) / sizeof((x)[0]))
#define member_size(t, f) (sizeof(((t*)0)->f))

/*
 * Some macros dedicated to integers.
 */

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif
#ifndef max
#define max(a,b) ((a) < (b) ? (b) : (a))
#endif

#define divup(n, d) (((n) + (d) - 1) / (d))
#define roundup(n, d) ((((n) + (d) - 1) / (d)) * (d))

#ifdef __cplusplus
}
#endif

#endif /* _USYS_STD_H */
