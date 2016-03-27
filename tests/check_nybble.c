#include <check.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

#include "../src/core/nybble.h"

static const uint8_t a1[] = { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0 };
static const uint8_t a2[] = { 0xFD, 0xFF };

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
