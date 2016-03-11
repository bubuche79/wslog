#include <stdlib.h>
#include <check.h>

#include "core/nybble.h"

static const uint8_t a1[] = { 0x12, 0x34 };

START_TEST(test_nybat)
{
	ck_assert_int_eq(nybat(a1, 0), 2);
	ck_assert_int_eq(nybat(a1, 1), 1);
	ck_assert_int_eq(nybat(a1, 2), 4);
	ck_assert_int_eq(nybat(a1, 3), 3);
}
END_TEST

START_TEST(test_nybtoi)
{
	ck_assert_int_eq(nybtoi(a1, 2, 0), 18);
	ck_assert_int_eq(nybtoi(a1, 4, 0), 13330);
	ck_assert_int_eq(nybtoi(a1, 2, 1), 65);
}
END_TEST

START_TEST(test_nybdtoi)
{
	ck_assert_int_eq(nybdtoi(a1, 2, 0), 12);
	ck_assert_int_eq(nybdtoi(a1, 4, 0), 3412);
	ck_assert_int_eq(nybdtoi(a1, 2, 1), 41);
}
END_TEST

Suite *nybble_suite(void)
{
	Suite *s;
	TCase *tc_core;
	TCase *tc_limits;

	s = suite_create("nybble");

	/* Core test cases */
	tc_core = tcase_create("core");

	tcase_add_test(tc_core, test_nybat);
	tcase_add_test(tc_core, test_nybtoi);
	tcase_add_test(tc_core, test_nybdtoi);
	suite_add_tcase(s, tc_core);

	/* Limits test cases */
	tc_limits = tcase_create("limits");
//	tcase_add_test(tc_limits, test_money_create_neg);
//	+    tcase_add_test(tc_limits, test_money_create_zero);
	suite_add_tcase(s, tc_limits);

	return s;
}

int
main()
{
	int number_failed;
	Suite *s;
	SRunner *sr;

	s = nybble_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
