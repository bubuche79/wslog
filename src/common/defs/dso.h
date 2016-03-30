#ifndef _USYS_DSO_H
#define _USYS_DSO_H

/*
 * Dynamic shared object definitions.
 */

#ifdef __cplusplus
extern "C" {
#endif

#if __GNUC__ >= 4
#define DSO_EXPORT	__attribute__ ((visibility("default")))
#define DSO_HIDE	__attribute__ ((visibility("hidden")))
#else
#define DSO_EXPORT
#define DSO_HIDE
#endif /* !(__GNUC__ >= 4) */

#ifdef __cplusplus
}
#endif

#endif /* _USYS_DSO_H */
