#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <time.h>
#include <check.h>

#include "libws/vantage/util.h"

#include "suites.h"

START_TEST(test_vantage_time)
{
	const time_t time = 1054884600;
	const uint8_t buf[] = { 0xc6, 0x06, 0xa2, 0x03 };
	uint8_t tmp[4];

	ck_assert_uint_eq(time, vantage_mktime(buf, 0, 1));

	vantage_localtime(tmp, time);
	ck_assert_mem_eq(buf, tmp, sizeof(buf));
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
	suite_add_tcase(s, tc_core);

	return s;
}
