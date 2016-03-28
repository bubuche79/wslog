#include <check.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

#include "../src/core/nybble.h"

#define ck_assert_failure(X, Y, Z) do { \
	ck_assert_int_eq(X, Y); \
	ck_assert_int_eq(errno, Z); \
} while (0)

static const uint8_t a1[] = { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0x13 };
static const uint8_t a2[] = { 0xFD, 0xFF, 0x36, 0x47, 0x58, 0x69, 0x7A, 0x8B, 0x9C };

START_TEST(test_nybat)
{
	ck_assert_int_eq(nybat(a1, 0), 2);
	ck_assert_int_eq(nybat(a1, 1), 1);
	ck_assert_int_eq(nybat(a1, 2), 4);
	ck_assert_int_eq(nybat(a1, 3), 3);
}
END_TEST

START_TEST(test_nybtol)
{
	ck_assert_int_eq(nybtol(a1, 2, 0), 18);
	ck_assert_int_eq(nybtol(a1, 4, 0), 13330);
	ck_assert_int_eq(nybtol(a1, 2, 1), 65);

	ck_assert_int_eq(nybtol(a2, 2, 0), -3);
	ck_assert_int_eq(nybtol(a2, 4, 0), -3);
	ck_assert_int_eq(nybtol(a2, 2, 1), -1);
}
END_TEST

START_TEST(test_nybdtol)
{
	ck_assert_int_eq(nybdtol(a1, 2, 0), 12);
	ck_assert_int_eq(nybdtol(a1, 4, 0), 3412);
	ck_assert_int_eq(nybdtol(a1, 2, 1), 41);
}
END_TEST

START_TEST(test_nybtol_limit)
{
	ck_assert_failure(nybtol(a1, 17, 0), LONG_MAX, ERANGE);
	ck_assert_failure(nybtol(a2, 17, 0), LONG_MIN, ERANGE);
}
END_TEST

START_TEST(test_nybdtol_limit)
{
	ck_assert_failure(nybdtol(a1, 1, 8), 0, EINVAL);
	ck_assert_failure(nybdtol(a1, 2, 8), 9, EINVAL);
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
	tcase_add_test(tc_core, test_nybtol);
	tcase_add_test(tc_core, test_nybdtol);
	suite_add_tcase(s, tc_core);

	/* Limits test cases */
	tc_limits = tcase_create("limits");
	tcase_add_test(tc_limits, test_nybtol_limit);
	tcase_add_test(tc_limits, test_nybdtol_limit);
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
