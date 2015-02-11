#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <time.h>
#include <check.h>

#include "libws/vantage/testing.c"
#include "libws/vantage/download.c"

#include "suites.h"

START_TEST(test_vantage_time)
{
	const time_t time = 1054884600;
	const uint8_t buf[] = { 0xc6, 0x06, 0xa2, 0x03 };

	setenv("TZ", "CET", 1);
	tzset();

	ck_assert_uint_eq(time, dmp_mktime(buf, 0));
}
END_TEST

/**
 * Check overflow of INT16_MAX
 */
START_TEST(test_vantage_rxcheck)
{
	const char buf[] = "-32079 252 1 699 131";
	struct vantage_rxck ck;

	scan_rxcheck(buf, &ck);

	ck_assert_uint_eq(33457, ck.pkt_recv);
	ck_assert_uint_eq(252, ck.pkt_missed);
	ck_assert_uint_eq(1, ck.resync);
	ck_assert_uint_eq(699, ck.pkt_in_row);
	ck_assert_uint_eq(131, ck.crc_ko);
}
END_TEST

Suite *
suite_vantage(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("vantage");

	/* Core test cases */
	tc_core = tcase_create("core");

	tcase_add_test(tc_core, test_vantage_time);
	tcase_add_test(tc_core, test_vantage_rxcheck);
	suite_add_tcase(s, tc_core);

	return s;
}
