#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <check.h>
#include <math.h>
#include <errno.h>

#include "libws/util.h"

#include "suites.h"

START_TEST(test_round_scale)
{
	ck_assert_double_eq(0.10, round_scale(0.1f, 2));
}
END_TEST

Suite *
suite_util(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("util");

	/* Core test cases */
	tc_core = tcase_create("core");

	tcase_add_test(tc_core, test_round_scale);
	suite_add_tcase(s, tc_core);

	return s;
}
