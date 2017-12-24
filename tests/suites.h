#ifndef _LIBWS_SUITES_H
#define _LIBWS_SUITES_H

#include <check.h>

#ifdef __cplusplus
extern "C" {
#endif

Suite *suite_util(void);
Suite *suite_nybble(void);
Suite *suite_crc_ccitt(void);
Suite *suite_aggregate(void);

Suite *suite_vantage(void);

#ifdef __cplusplus
}
#endif

#endif	/* _LIBWS_SUITES_H */
