#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <time.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sqlite3.h>

#include "libws/cli.h"

enum tm_unit {
	SECOND,
	MINUTE,
	HOUR,
	DAY,
	MONTH,
	YEAR
};

static int help = 0;
static const char *dbfile = "/var/lib/wslog/wslogd.db";

static const char *since = NULL;
static const char *until = NULL;
const char *group_by = "day";

static struct opt ic_omm_opts[] = {
	OPT_BIT('h', "help", &help, "display help", 1),
	OPT_STRING('f', "file", &dbfile, "sqlite data file"),
	OPT_STRING(0, "since", &since, "logs since a specific date"),
	OPT_STRING(0, "until", &until, "logs until a specific date"),
	OPT_STRING('g', "group-by", &group_by, "group by day, month or year"),
	OPT_END()
};

static time_t
strtotime(const char *s, enum tm_unit unit)
{
	const char *r;
	struct tm tm;
	const char *fmt;

	if (unit == DAY) {
		fmt = "%Y-%m-%d";
	} else if (unit == MONTH) {
		fmt = "%Y-%m";
	} else if (unit == YEAR) {
		fmt = "%Y";
	} else {
		goto error;
	}

	if ((r = strptime(s, fmt, &tm)) == NULL || *r) {
		goto error;
	}

	tm.tm_isdst = -1;

	return mktime(&tm);

error:
	errno = EINVAL;
	return -1;
}

static time_t
strtounit(const char *s)
{
	enum tm_unit res;

	if (!strcmp("day", s)) {
		res = DAY;
	} else if (!strcmp("month", s)) {
		res = MONTH;
	} else if (!strcmp("year", s)) {
		res = YEAR;
	} else {
		errno = EINVAL;
		res = -1;
	}

	return res;
}

static time_t
trunct(const time_t *timep, enum tm_unit unit)
{
	struct tm tmin, tmout;

	gmtime_r(timep, &tmin);
	memset(&tmout, 0, sizeof(tmout));

	if (unit <= SECOND) {
		tmout.tm_sec = tmin.tm_sec;
	}
	if (unit <= MINUTE) {
		tmout.tm_min = tmin.tm_min;
	}
	if (unit <= HOUR) {
		tmout.tm_hour = tmin.tm_hour;
	}
	if (unit <= DAY) {
		tmout.tm_mday = tmin.tm_mday;
	}
	if (unit <= MONTH) {
		tmout.tm_mon = tmin.tm_mon;
	}
	if (unit <= YEAR) {
		tmout.tm_year = tmin.tm_year;
	}

	return timegm(&tmout);
}

static void
sqlite_trunct(sqlite3_context* ctx, int argc, sqlite3_value **argv)
{
	time_t v, res;
	int unit;

	v = sqlite3_value_int64(argv[0]);
	unit = sqlite3_value_int(argv[1]);

	res = trunct(&v, unit);

	sqlite3_result_int64(ctx, res);
}


static int
sqlite_open(sqlite3 **db, const char *filename)
{
	sqlite3_initialize();
	sqlite3_open_v2(filename, db, SQLITE_OPEN_READONLY, NULL);

	sqlite3_create_function(*db, "date_trunc", 2,
			SQLITE_UTF8 | SQLITE_DETERMINISTIC,
			NULL, sqlite_trunct, NULL, NULL);

	return SQLITE_OK;
}

static int
sqlite_prepare(sqlite3 *db, sqlite3_stmt **query)
{
	int ret;

	const char *sql =
		"with subq AS ( "
		  "select date_trunc(time - 1, 2) as time, "
		    "min(temp) as lo_temp, "
		    "max(temp) as hi_temp, "
		    "sum(rain_fall) as rain_fall, "
		    "max(hi_wind_speed) as hi_wind_speed "
		  "from ws_archive "
		  "where time >= ? and time < ? "
		  "group by date_trunc(time - 1, 2) "
		") "
		"select q1.time, "
		  "q1.lo_temp, "
		  "q2.hi_temp, "
		  "q2.rain_fall, "
		  "q3.hi_wind_speed "
		"from ( "
		  "select date_trunc(time + 3600*6, 3) as time, "
		    "min(lo_temp) as lo_temp "
		  "from subq "
		  "group by date_trunc(time + 3600*6, 3) "
		") q1, ( "
		  "select date_trunc(time - 3600*6, 3) as time, "
		    "max(hi_temp) as hi_temp, "
		    "sum(rain_fall) as rain_fall "
		  "from subq "
		  "group by date_trunc(time - 3600*6, 3) "
		") q2, ( "
		  "select date_trunc(time, 3) as time, "
		    "max(hi_wind_speed) as hi_wind_speed "
		  "from subq "
		  "group by date_trunc(time, 3) "
		") q3 "
		"where q1.time = q2.time "
		  "and q3.time = q1.time "
		"order by q1.time";

	ret = sqlite3_prepare_v2(db, sql, -1, query, NULL);
	if (ret != SQLITE_OK) {
//		sqlite_log("sqlite3_prepare_v2", ret);
//		goto error;
	}

	return ret;
}

int
cmd_ic_omm(int argc, const char **argv)
{
	int arg_idx;

	time_t after = 0, before = 0;
	enum tm_unit by;

	/* Parse arguments */
	arg_idx = opt_parse(argc, argv, ic_omm_opts);

	if (arg_idx != argc) {
		opt_fprintf(stderr, "wslog-ic-omm", ic_omm_opts);
		goto error;
	}

	if (help) {
		opt_fprintf(stderr, "wslog-ic-omm", ic_omm_opts);
		return 0;
	}

	if ((by = strtounit(group_by)) == -1) {
		fprintf(stderr, "Invalid grouping option: %s\n", group_by);
		goto error;
	}

	if (since && (after = strtotime(since, by)) == -1) {
		fprintf(stderr, "Invalid date format: %s\n", since);
		goto error;
	}
	if (until && (before = strtotime(until, by)) == -1) {
		fprintf(stderr, "Invalid date format: %s\n", until);
		goto error;
	}

	sqlite3 *db;
	sqlite3_stmt *query;

	sqlite_open(&db, dbfile);
	sqlite_prepare(db, &query);

	sqlite3_bind_int64(query, 1, after);
	sqlite3_bind_int64(query, 2, before);

	while (sqlite3_step(query) == SQLITE_ROW) {
		char outstr[200];
		struct tm outtm;

		time_t v = sqlite3_column_int64(query, 0);

		double lo_temp = sqlite3_column_double(query, 1);
		double hi_temp = sqlite3_column_double(query, 2);
		double rain_fall = sqlite3_column_double(query, 3);
		double hi_wind_speed = sqlite3_column_double(query, 4);

		gmtime_r(&v, &outtm);
		strftime(outstr, sizeof(outstr), "%F", &outtm);

		printf("%s,%.1f,%.1f,%.1f,%.1f\n", outstr,
				lo_temp, hi_temp,
				rain_fall, hi_wind_speed * 3.6);
	}

	return 0;

error:
	return 1;
}
