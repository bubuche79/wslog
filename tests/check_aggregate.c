#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <check.h>
#include <errno.h>

#include "defs/std.h"

#include "libws/aggregate.h"

#define ck_assert_double(X, OP, Y) \
	do { \
		double _ck_x = (X); \
		double _ck_y = (Y); \
		ck_assert_msg(_ck_x == _ck_y, "Assertion '%s' failed: %s == %f, %s == %f", #X" "#OP" "#Y, #X, _ck_x, #Y, _ck_y); \
	} while (0)

#define ck_assert_double_eq(X, Y) \
	ck_assert_double(X, ==, Y)

static const int a1[] = { 1, 3, 6, 8 };
static const int a2[] = { 8, 6, 5, 2, 0 };
static const int a3[] = { -1, 0, 4 };

static double
aggr(const int *arr, size_t len, enum aggr_type type)
{
	int i;
	double val;
	struct aggr_data s;

	aggr_init(&s, type);
	for (i = 0; i < len; i++) {
		aggr_update(&s, arr[i]);
	}
	aggr_finalize(&s, &val);

	return val;
}

static double
aggr_min(const int *arr, size_t len)
{
	return aggr(arr, len, AGGR_MIN);
}

static double
aggr_max(const int *arr, size_t len)
{
	return aggr(arr, len, AGGR_MAX);
}

static double
aggr_sum(const int *arr, size_t len)
{
	return aggr(arr, len, AGGR_SUM);
}

static double
aggr_avg(const int *arr, size_t len)
{
	return aggr(arr, len, AGGR_AVG);
}

static double
aggr_count(const int *arr, size_t len)
{
	return aggr(arr, len, AGGR_COUNT);
}

START_TEST(test_min)
{
	ck_assert_double_eq(aggr_min(a1, array_size(a1)), 1.0);
	ck_assert_double_eq(aggr_min(a2, array_size(a2)), 0.0);
	ck_assert_double_eq(aggr_min(a3, array_size(a3)), -1.0);
}
END_TEST

START_TEST(test_max)
{
	ck_assert_double_eq(aggr_max(a1, array_size(a1)), 8.0);
	ck_assert_double_eq(aggr_max(a2, array_size(a2)), 8.0);
	ck_assert_double_eq(aggr_max(a3, array_size(a3)), 4.0);
}
END_TEST

START_TEST(test_sum)
{
	ck_assert_double_eq(aggr_sum(a1, array_size(a1)), 18.0);
	ck_assert_double_eq(aggr_sum(a2, array_size(a2)), 21.0);
	ck_assert_double_eq(aggr_sum(a3, array_size(a3)), 3.0);
}
END_TEST

START_TEST(test_avg)
{
	ck_assert_double_eq(aggr_avg(a1, array_size(a1)), 4.5);
	ck_assert_double_eq(aggr_avg(a2, array_size(a2)), 4.2);
	ck_assert_double_eq(aggr_avg(a3, array_size(a3)), 1.0);
}
END_TEST

START_TEST(test_count)
{
	ck_assert_double_eq(aggr_count(a1, array_size(a1)), 4.0);
	ck_assert_double_eq(aggr_count(a2, array_size(a2)), 5.0);
	ck_assert_double_eq(aggr_count(a3, array_size(a3)), 3.0);
}
END_TEST

Suite *
aggregate_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("aggregate");

	/* Core test cases */
	tc_core = tcase_create("core");

	tcase_add_test(tc_core, test_min);
	tcase_add_test(tc_core, test_max);
	tcase_add_test(tc_core, test_sum);
	tcase_add_test(tc_core, test_avg);
	tcase_add_test(tc_core, test_count);
	suite_add_tcase(s, tc_core);

	return s;
}

int
main(void)
{
	int number_failed;
	Suite *s;
	SRunner *sr;

	s = aggregate_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? 0 : 1;
}
