#include <check.h>

#include "libws/cli.c"
#include "libws/defs.h"

#include "suites.h"

START_TEST(test_opt_parse)
{
	int ret;
	int bl = 0, bs = 0, packed = 0;
	int il = 0, is = 0, ipacked = 0;
	char *s = NULL;
	char *cmd = NULL;

	struct opt opts[] = {
		OPT_BIT(0, "bit", &bl, "bit data", 1),
		OPT_BIT('b', NULL, &bs, "bit data", 3),
		OPT_BIT('P', NULL, &packed, "bit data", 2),
		OPT_INT(0, "integer", &il, "integer data"),
		OPT_INT('i', NULL, &is, "integer data"),
		OPT_INT('X', NULL, &ipacked, "integer data"),
		OPT_STRING(0, "string", &s, "string data"),
		OPT_COMMAND("test", &cmd, "subcommand"),
		OPT_END()
	};

	const char *argv[] = {
		"program",
		"--bit",
		"-bPX66",
		"--integer", "2",
		"-i", "5",
		"--string", "my val",
		"test",
		"first_arg"
	};

	ret = opt_parse(array_size(argv), argv, opts);

	ck_assert_int_eq(10, ret);
	ck_assert_int_eq(1, bl);
	ck_assert_int_eq(3, bs);
	ck_assert_int_eq(2, packed);
	ck_assert_int_eq(66, ipacked);
	ck_assert_int_eq(2, il);
	ck_assert_int_eq(5, is);
	ck_assert_str_eq("my val", s);
	ck_assert_str_eq("test", cmd);
}
END_TEST

Suite *
suite_cli(void)
{
	Suite *s;
	TCase *tc_cli;

	s = suite_create("cli");

	/* Core test cases */
	tc_cli = tcase_create("core");
	tcase_add_test(tc_cli, test_opt_parse);

	suite_add_tcase(s, tc_cli);

	return s;
}
