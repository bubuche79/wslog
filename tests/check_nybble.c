#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <check.h>
#include <stdint.h>
#include <limits.h>
#include <errno.h>

#include "libws/nybble.h"

#define ck_assert_err(X, Y, Z) \
	do { \
		ck_assert_uint_eq(X, Y); \
		ck_assert_uint_eq(errno, Z); \
	} while (0)

#define ck_assert_ultonyb(buf, nnyb, off, v, base) \
	do { \
		ultonyb(buf, nnyb, off, v, base); \
		ck_assert_uint_eq(nybtoul(buf, nnyb, off, base), v); \
	} while (0)

static const uint8_t a1[] = { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0x13 };
static const uint8_t a2[] = { 0xFD, 0xFF, 0x36, 0x47, 0x58, 0x69, 0x7A, 0x8B, 0x9C };

START_TEST(test_nybget)
{
	ck_assert_uint_eq(nybget(a1, 0), 2);
	ck_assert_uint_eq(nybget(a1, 1), 1);
	ck_assert_uint_eq(nybget(a1, 2), 4);
	ck_assert_uint_eq(nybget(a1, 3), 3);
}
END_TEST

START_TEST(test_nybset_auto)
{
	int i;
	size_t off;
	uint8_t a[4];

	for (off = 0; off < 2 * sizeof(a); off++) {
		for (i = 0; i < 16; i++) {
			nybset(a, off, i);
			ck_assert_uint_eq(nybget(a, off), i);
		}
	}
}
END_TEST

START_TEST(test_nybtoul)
{
	/* Base 16 */
	ck_assert_uint_eq(nybtoul(a1, 2, 0, 16), 18);
	ck_assert_uint_eq(nybtoul(a1, 4, 0, 16), 13330);
	ck_assert_uint_eq(nybtoul(a1, 2, 1, 16), 65);

	ck_assert_uint_eq(nybtoul(a2, 2, 0, 16), 253);
	ck_assert_uint_eq(nybtoul(a2, 4, 0, 16), 65533);
	ck_assert_uint_eq(nybtoul(a2, 2, 1, 16), 255);

	/* Base 10 */
	ck_assert_uint_eq(nybtoul(a1, 2, 0, 10), 12);
	ck_assert_uint_eq(nybtoul(a1, 4, 0, 10), 3412);
	ck_assert_uint_eq(nybtoul(a1, 2, 1, 10), 41);
}
END_TEST

START_TEST(test_ultonyb)
{
	uint8_t a[4];

	/* Base 16 */
	ck_assert_ultonyb(a, 2, 0, 18, 16);
	ck_assert_ultonyb(a, 3, 0, 18, 16);
	ck_assert_ultonyb(a, 2, 1, 65, 16);
	ck_assert_ultonyb(a, 2, 1, 255, 16);
	ck_assert_ultonyb(a, 3, 1, 255, 16);

	/* Base 10 */
	ck_assert_ultonyb(a, 4, 0, 3412, 10);
}
END_TEST

START_TEST(test_nybtoul_erange)
{
	ck_assert_err(nybtoul(a1, 17, 0, 16), ULONG_MAX, ERANGE);
}
END_TEST

START_TEST(test_nybtoul_einval)
{
	ck_assert_err(nybtoul(a1, 1, 8, 10), 0, EINVAL);
	ck_assert_err(nybtoul(a1, 2, 8, 10), 9, EINVAL);
}
END_TEST

Suite *
nybble_suite(void)
{
	Suite *s;
	TCase *tc_core;
	TCase *tc_limits;

	s = suite_create("nybble");

	/* Core test cases */
	tc_core = tcase_create("core");

	tcase_add_test(tc_core, test_nybget);
	tcase_add_test(tc_core, test_nybset_auto);
	tcase_add_test(tc_core, test_nybtoul);
	tcase_add_test(tc_core, test_ultonyb);
	suite_add_tcase(s, tc_core);

	/* Limits test cases */
	tc_limits = tcase_create("limits");
	tcase_add_test(tc_limits, test_nybtoul_erange);
	tcase_add_test(tc_limits, test_nybtoul_einval);
	suite_add_tcase(s, tc_limits);

	return s;
}

int
main(void)
{
	int number_failed;
	Suite *s;
	SRunner *sr;

	s = nybble_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? 0 : 1;
}
