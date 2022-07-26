#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <check.h>

#include "libws/crc_ccitt.h"

#include "suites.h"

START_TEST(test_crc_ccitt)
{
	uint8_t a1[] = { 0xC6, 0xCE, 0xA2, 0x03 };
	uint8_t a2[] = { 0xC6, 0xCE, 0xA2, 0x03, 0xE2, 0xB4 };
	uint8_t a3[] = { 0xE2, 0xB4 };

	ck_assert_uint_eq(0xE2B4, ws_crc_ccitt(0, a1, sizeof(a1)));
	ck_assert_uint_eq(0, ws_crc_ccitt(0, a2, sizeof(a2)));
	ck_assert_uint_eq(0, ws_crc_ccitt(ws_crc_ccitt(0, a1, sizeof(a1)), a3, sizeof(a3)));
}
END_TEST

Suite *
suite_crc_ccitt(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("crc_ccitt");

	/* Core test cases */
	tc_core = tcase_create("core");
	tcase_add_test(tc_core, test_crc_ccitt);

	suite_add_tcase(s, tc_core);

	return s;
}
