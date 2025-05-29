#ifndef _LIBWS_SUITES_H
#define _LIBWS_SUITES_H

#include <check.h>

#ifndef ck_assert_double_eq
#define ck_assert_double_eq(X, Y) ck_assert_int_eq(1000*(X), 1000*(Y))
#endif

#ifdef __cplusplus
extern "C" {
#endif

Suite *suite_aggregate(void);
Suite *suite_cli(void);
Suite *suite_crc_ccitt(void);
Suite *suite_nybble(void);
Suite *suite_util(void);

Suite *suite_vantage(void);

#ifdef __cplusplus
}
#endif

#endif	/* _LIBWS_SUITES_H */
