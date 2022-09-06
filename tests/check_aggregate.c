#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <check.h>
#include <errno.h>

#include "libws/defs.h"
#include "libws/aggregate.h"

#include "suites.h"

#define ck_assert_avgdeg(arr, len, rc, v) \
	do { \
		int i, err; \
		double actual; \
		struct aggr abuf; \
		aggr_init_avgdeg(&abuf); \
		for (i = 0; i < len; i++) { \
			aggr_add(&abuf, arr[i]); \
		} \
		err = aggr_finish(&abuf, &actual); \
		ck_assert_int_eq(rc, err); \
		ck_assert_double_eq_tol(v, actual, 0.01); \
	} while (0)

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

static int
aggr_avgdeg(const double *arr, size_t len, double *v)
{
	int i;
	struct aggr abuf;

	aggr_init_avgdeg(&abuf);

	for (i = 0; i < len; i++) {
		aggr_add(&abuf, arr[i]);
	}

	return aggr_finish(&abuf, v);
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

START_TEST(test_avgdeg)
{
	double set1[] = { 30, 330, 30 };
	double set2[] = { 90, 180, 270, 360 };
	double set3[] = { 10, 20, 30 };

	ck_assert_avgdeg(set1, 0, -1, 0);
	ck_assert_avgdeg(set1, 1, 0, 30);
	ck_assert_avgdeg(set1, 2, 0, 360);

	ck_assert_avgdeg(set2, 4, 0, 270);
	ck_assert_avgdeg(set3, 3, 0, 20);
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
suite_aggregate(void)
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
	tcase_add_test(tc_core, test_avgdeg);
	tcase_add_test(tc_core, test_count);

	suite_add_tcase(s, tc_core);

	return s;
}
