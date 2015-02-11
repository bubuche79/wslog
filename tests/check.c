#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <check.h>

#include "suites.h"

int
main(void)
{
	int number_failed;
	SRunner *sr;

	sr = srunner_create(NULL);

	srunner_add_suite(sr, suite_aggregate());
	srunner_add_suite(sr, suite_crc_ccitt());
	srunner_add_suite(sr, suite_nybble());
	srunner_add_suite(sr, suite_util());

	srunner_add_suite(sr, suite_vantage());

	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	return (number_failed == 0) ? 0 : 1;
}
