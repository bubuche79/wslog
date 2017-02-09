#ifndef _CORE_ERR_H
#define _CORE_ERR_H

/*
 * Formatted error messages.
 */

#ifdef __cplusplus
extern "C" {
#endif

void die(int status, const char *fmt, /* args */ ...)
#if __GNUC__ >= 4
    __attribute__ ((format (printf, 2, 3)))
#endif
    ;

#ifdef __cplusplus
}
#endif

#endif /* _CORE_ERR_H */
