#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <check.h>
#include <math.h>
#include <errno.h>

#include "libws/util.h"

#define ck_assert_double(X, OP, Y) \
	do { \
		double _ck_x = (X); \
		double _ck_y = (Y); \
		ck_assert_msg(_ck_x == _ck_y, "Assertion '%s' failed: %s == %f, %s == %f", #X" "#OP" "#Y, #X, _ck_x, #Y, _ck_y); \
	} while (0)

#define ck_assert_double_eq(X, Y) \
	ck_assert_double(X, ==, Y)

START_TEST(test_round_scale)
{
	ck_assert_double_eq(0.10, round_scale(0.1f, 2));
}
END_TEST

Suite *
util_suite(void)
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

int
main(void)
{
	int number_failed;
	Suite *s;
	SRunner *sr;

	s = util_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? 0 : 1;
}
