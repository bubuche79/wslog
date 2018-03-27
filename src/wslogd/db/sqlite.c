#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <sqlite3.h>

#include "libws/defs.h"
#include "libws/util.h"

#include "board.h"
#include "conf.h"
#include "wslogd.h"
#include "sqlite.h"

#define SQL_MAX		1024
#define SQL_TABLE	"ws_archive"
#define SQL_CREATE	"/usr/share/wslog/sqlite.sql"

#define bufsz(buf, p, len) ((len) - ((p) - (buf)))

struct ws_db
{
	const char *col_name;
	int col_type;
	int (*get) (const struct ws_archive *, double *);
	int (*set) (struct ws_archive *, double);
	int ignore;
};

static sqlite3 *db;			/* Database handle */
static sqlite3_stmt *stmt;		/* Insert prepared statement */

static const struct ws_db columns[] =
{
	{ "time", SQLITE_INTEGER },
	{ "interval", SQLITE_INTEGER },
	{ "barometer", SQLITE_FLOAT, ws_get_barometer, ws_set_barometer },
	{ "temp", SQLITE_FLOAT, ws_get_temp, ws_set_temp },
	{ "lo_temp", SQLITE_FLOAT, ws_get_lo_temp, ws_set_lo_temp },
	{ "hi_temp", SQLITE_FLOAT, ws_get_hi_temp, ws_set_hi_temp },
	{ "humidity", SQLITE_INTEGER, ws_get_humidity, ws_set_humidity },
	{ "avg_wind_speed", SQLITE_FLOAT, ws_get_wind_speed, ws_set_wind_speed },
	{ "avg_wind_dir", SQLITE_INTEGER, ws_get_wind_dir, ws_set_wind_dir },
	{ "wind_samples", SQLITE_INTEGER, ws_get_wind_samples, ws_set_wind_samples },
	{ "hi_wind_speed", SQLITE_FLOAT, ws_get_hi_wind_speed, ws_set_hi_wind_speed },
	{ "hi_wind_dir", SQLITE_INTEGER, ws_get_hi_wind_dir, ws_set_hi_wind_dir },
	{ "rain_fall", SQLITE_FLOAT, ws_get_rain, ws_set_rain },
	{ "hi_rain_rate", SQLITE_FLOAT, ws_get_hi_rain_rate, ws_set_hi_rain_rate },
	{ "dew_point", SQLITE_FLOAT, ws_get_dew_point, ws_set_dew_point },
	{ "windchill", SQLITE_FLOAT, ws_get_windchill, ws_set_windchill },
	{ "heat_index", SQLITE_FLOAT, ws_get_heat_index, ws_set_heat_index },
	{ "in_temp", SQLITE_FLOAT, ws_get_in_temp, ws_set_in_temp },
	{ "in_humidity", SQLITE_INTEGER, ws_get_in_humidity, ws_set_in_humidity }
};

static size_t columns_nel = array_size(columns);

static void
sqlite_log(const char *fn, int code)
{
	syslog(LOG_ERR, "%s: %s (%d)", fn, sqlite3_errstr(code), code);
}

static char *
sql_columns(char *buf, size_t len)
{
	int i;
	char *p = buf;

	for (i = 0; i < columns_nel; i++) {
		if (i > 0) {
			p = stpncpy(p, ", ", bufsz(buf, p, len));
		}

		p = stpncpy(p, columns[i].col_name, bufsz(buf, p, len));
	}

	return p;
}

static ssize_t
sql_create(char *buf, size_t len)
{
	ssize_t sz;
	const char* sqlfile = SQL_CREATE;

	if ((sz = ws_read_all(sqlfile, buf, len)) == -1) {
		syslog(LOG_ERR, "ws_read_all %s: %m", sqlfile);
		goto error;
	}

	buf[sz] = 0;

	return sz;

error:
	return -1;
}

static ssize_t
sql_insert(char *buf, size_t len)
{
	int i;
	char *p = buf;

	p = stpncpy(p, "INSERT INTO " SQL_TABLE " (", bufsz(buf, p, len));

	p = sql_columns(p, bufsz(buf, p, len));

	p = stpncpy(p, ") VALUES (", bufsz(buf, p, len));

	for (i = 0; i < columns_nel; i++) {
		if (i > 0) {
			p = stpncpy(p, ", ", bufsz(buf, p, len));
		}

		p = stpncpy(p, "?", bufsz(buf, p, len));
	}

	p = stpncpy(p, ")", bufsz(buf, p, len));

	return p - buf;
}

static size_t
sql_select(char *buf, size_t len)
{
	char *p = buf;

	p = stpncpy(p, "SELECT ", bufsz(buf, p, len));
	p = sql_columns(p, bufsz(buf, p, len));
	p = stpncpy(p, " FROM " SQL_TABLE " ORDER BY time DESC LIMIT ?", bufsz(buf, p, len));

	return p - buf;
}

static int
sqlite_step()
{
	int ret;
	int reset = 1;

	if ((ret = sqlite3_step(stmt)) != SQLITE_DONE) {
		sqlite_log("sqlite3_step", ret);
		goto error;
	}

	reset = 0;

	if ((ret = sqlite3_reset(stmt)) != SQLITE_OK) {
		sqlite_log("sqlite3_reset", ret);
		goto error;
	}

	return 0;

error:
	if (reset) {
		(void) sqlite3_reset(stmt);
	}
	return -1;
}

static int
sqlite_stmt_insert(const struct ws_archive *p)
{
	int ret;
	int i, bind_index;

	/* Bind variables */
	bind_index = 1;

	ret = sqlite3_bind_int64(stmt, bind_index++, p->time);
	if (SQLITE_OK != ret) {
		sqlite_log("sqlite3_bind_int64", ret);
		goto error;
	}
	ret = sqlite3_bind_int(stmt, bind_index++, p->interval);
	if (SQLITE_OK != ret) {
		sqlite_log("sqlite3_bind_int", ret);
		goto error;
	}

	for (i = 2; i < columns_nel; i++) {
		double value;

		if (columns[i].get(p, &value) == 0) {
			switch (columns[i].col_type) {
			case SQLITE_INTEGER:
				ret = sqlite3_bind_int(stmt, bind_index, value);
				break;
			case SQLITE_FLOAT:
				ret = sqlite3_bind_double(stmt, bind_index, round_scale(value, 2));
				break;
			default:
				break;
			}
		} else {
			ret = sqlite3_bind_null(stmt, bind_index);
		}

		if (SQLITE_OK != ret) {
			sqlite_log("sqlite3_bind_xx", ret);
			goto error;
		}

		bind_index++;
	}

	/* Execute statement */
	if (!dry_run) {
		if (sqlite_step() == -1) {
			goto error;
		}
	}

	return 0;

error:
	return -1;
}

static int
sqlite_exec(const char *stmt)
{
	int ret;

	ret = sqlite3_exec(db, stmt, NULL, NULL, NULL);
	if (ret != SQLITE_OK) {
		sqlite_log("sqlite3_exec", ret);
		return -1;
	}

	return 0;
}

int
sqlite_init(void)
{
	int sz, ret;
	struct stat sbuf;
	int oflag = 0;
	char sqlbuf[SQL_MAX];
	const char *dbfile = confp->archive.sqlite.db;

	/* Clear */
	db = NULL;
	stmt = NULL;

	ret = sqlite3_initialize();
	if (ret != SQLITE_OK) {
		sqlite_log("sqlite3_initialize", ret);
		goto error;
	}

	/* Open database */
	ret = stat(dbfile, &sbuf);
	if (ret == -1) {
		if (errno == ENOENT) {
			oflag |= SQLITE_OPEN_CREATE;
		} else {
			syslog(LOG_ERR, "stat %s: %m", dbfile);
			goto error;
		}
	}

	oflag |= SQLITE_OPEN_READWRITE;

	ret = sqlite3_open_v2(dbfile, &db, oflag, NULL);
	if (ret != SQLITE_OK) {
		syslog(LOG_ERR, "sqlite3_open_v2 %s: %s", dbfile, sqlite3_errstr(ret));
		goto error;
	}

	/* Create schema */
	if (oflag & SQLITE_OPEN_CREATE) {
		if (sql_create(sqlbuf, sizeof(sqlbuf)) == -1) {
			goto error;
		}

		ret = sqlite3_exec(db, sqlbuf, NULL, NULL, NULL);
		if (ret != SQLITE_OK) {
			sqlite_log("sqlite3_exec", ret);
			goto error;
		}
	}

	/* Prepare statement */
	sz = sql_insert(sqlbuf, sizeof(sqlbuf));

	ret = sqlite3_prepare_v2(db, sqlbuf, sz, &stmt, NULL);
	if (ret != SQLITE_OK) {
		sqlite_log("sqlite3_prepare_v2", ret);
		goto error;
	}

	syslog(LOG_INFO, "sqlite %s: connected", dbfile);

	return 0;

error:
	if (db != NULL) {
		(void) sqlite3_close_v2(db);
		db = NULL;
	}
	return -1;
}

int
sqlite_destroy(void)
{
	int ret;
	int status;

	status = 0;

	ret = sqlite3_finalize(stmt);
	if (ret != SQLITE_OK) {
		status = -1;
		sqlite_log("sqlite3_finalize", ret);
	}

	if (db != NULL) {
		ret = sqlite3_close_v2(db);
		if (ret != SQLITE_OK) {
			status = -1;
			sqlite_log("sqlite3_close_v2", ret);
		}
	}

	ret = sqlite3_shutdown();
	if (ret != SQLITE_OK) {
		status = -1;
		sqlite_log("sqlite3_shutdown", ret);
	}

	return status;
}

int
sqlite_begin()
{
	return sqlite_exec("BEGIN");
}

int
sqlite_commit()
{
	return sqlite_exec("COMMIT");
}

int
sqlite_rollback()
{
	return sqlite_exec("ROLLBACK");
}

ssize_t
sqlite_insert(const struct ws_archive *p, size_t nel)
{
	size_t i;

	for (i = 0; i < nel; i++) {
		if (sqlite_stmt_insert(&p[i]) == -1) {
			goto error;
		}
	}

	return i;

error:
	return -1;
}

static void
fetch_loop_columns(struct ws_archive *p, sqlite3_stmt *stmt, int col_index)
{
	int i;

	p->wl_mask = 0;

	for (i = 2; i < columns_nel; i++) {
		int type;
		double value;

		type = sqlite3_column_type(stmt, col_index);

		switch (type) {
		case SQLITE_INTEGER:
			value = sqlite3_column_int(stmt, col_index);
			columns[i].set(p, value);
			break;
		case SQLITE_FLOAT:
			value = sqlite3_column_double(stmt, col_index);
			columns[i].set(p, value);
			break;
		case SQLITE_NULL:
			break;
		}

		col_index++;
	}
}

ssize_t
sqlite_select_last(struct ws_archive *p, size_t nel)
{
	int i, ret;
	sqlite3_stmt *query;
	char sqlbuf[SQL_MAX];

	/* Prepare query */
	query = NULL;
	sql_select(sqlbuf, sizeof(sqlbuf));

	ret = sqlite3_prepare_v2(db, sqlbuf, -1, &query, NULL);
	if (ret != SQLITE_OK) {
		sqlite_log("sqlite3_prepare_v2", ret);
		goto error;
	}

	/* Bind parameters */
	ret = sqlite3_bind_int(query, 1, nel);
	if (ret != SQLITE_OK) {
		sqlite_log("sqlite3_bind_int", ret);
		goto error;
	}

	/* Execute and process result set */
	i = 0;

	while(i < nel && sqlite3_step(query) == SQLITE_ROW) {
		int col_idx = 0;

		p[i].time = sqlite3_column_int64(query, col_idx++);
		p[i].interval = sqlite3_column_int(query, col_idx++);

		fetch_loop_columns(&p[i], query, col_idx);

		i++;
	}

	ret = sqlite3_reset(query);
	if (ret != SQLITE_OK) {
		sqlite_log("sqlite3_reset", ret);
		goto error;
	}
	ret = sqlite3_finalize(query);
	if (ret != SQLITE_OK) {
		sqlite_log("sqlite3_finalize", ret);
		goto error;
	}

	return i;

error:
	(void) sqlite3_reset(query);
	(void) sqlite3_finalize(query);

	return -1;
}
