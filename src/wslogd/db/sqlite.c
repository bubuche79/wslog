#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <syslog.h>
#include <sqlite3.h>

#include "defs/std.h"
#include "libws/util.h"

#include "board.h"
#include "conf.h"
#include "wslogd.h"
#include "sqlite.h"

#define SQL_CREATE \
	"CREATE TABLE ws_archive ( " \
	"  time INTEGER NOT NULL, " \
	"  interval INTEGER NOT NULL, " \
	"  pressure REAL, " \
	"  barometer REAL, " \
	"  temp REAL, " \
	"  humidity INTEGER, " \
	"  wind_speed REAL, " \
	"  wind_dir INTEGER, " \
	"  wind_gust_speed REAL, " \
	"  wind_gust_dir INTEGER, " \
	"  rain REAL, " \
	"  rain_rate REAL, " \
	"  dew_point REAL, " \
	"  windchill REAL, " \
	"  heat_index REAL, " \
	"  temp_in REAL, " \
	"  humidity_in INTEGER, " \
	"  PRIMARY KEY (time)" \
	") "

#define SQL_INSERT \
	"INSERT INTO ws_archive " \
	"VALUES " \
	"(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?) "

#define SQL_SELECT \
	"SELECT * FROM ws_archive ORDER BY time DESC LIMIT ?"

struct ws_db
{
	const char *col_name;
	int col_type;
	int (*get) (const struct ws_loop *, double *);
	int (*set) (struct ws_loop *, double);
};

static sqlite3 *db;
static sqlite3_stmt *stmt;

static struct ws_db columns[] =
{
	{ "pressure", SQLITE_FLOAT, ws_get_pressure, ws_set_pressure },
//	{ "altimeter", SQLITE_FLOAT, ws_get_altimeter, ws_set_altimeter },
	{ "barometer", SQLITE_FLOAT, ws_get_barometer, ws_set_barometer },
	{ "temp", SQLITE_FLOAT, ws_get_temp, ws_set_temp },
	{ "humidity", SQLITE_INTEGER, ws_get_humidity, ws_set_humidity },
	{ "wind_speed", SQLITE_FLOAT, ws_get_wind_speed, ws_set_wind_speed },
	{ "wind_dir", SQLITE_INTEGER, ws_get_wind_dir, ws_set_wind_dir },
	{ "wind_gust_speed", SQLITE_FLOAT, ws_get_wind_gust_speed, ws_set_wind_gust_speed },
	{ "wind_gust_dir", SQLITE_INTEGER, ws_get_wind_gust_dir, ws_set_wind_gust_dir },
	{ "rain", SQLITE_FLOAT, ws_get_rain, ws_set_rain },
	{ "rain_rate", SQLITE_FLOAT, ws_get_rain_rate, ws_set_rain_rate },
	{ "dew_point", SQLITE_FLOAT, ws_get_dew_point, ws_set_dew_point },
	{ "windchill", SQLITE_FLOAT, ws_get_windchill, ws_set_windchill },
	{ "heat_index", SQLITE_FLOAT, ws_get_heat_index, ws_set_heat_index },
	{ "temp_in", SQLITE_FLOAT, ws_get_temp_in, ws_set_temp_in },
	{ "humidity_in", SQLITE_INTEGER, ws_get_humidity_in, ws_set_humidity_in }
};

static size_t columns_nel = array_size(columns);

static void
sqlite_log(const char *fn, int code)
{
	syslog(LOG_ERR, "%s: %s", fn, sqlite3_errstr(code));
}

static int
stmt_insert(const struct ws_archive *p)
{
	int ret;
	int i, bind_index;

	ret = sqlite3_reset(stmt);
	if (SQLITE_OK != ret) {
		sqlite_log("sqlite3_reset", ret);
		goto error;
	}

	/* Bind variables */
	bind_index = 1;

	ret = sqlite3_bind_int64(stmt, bind_index++, p->data.time);
	if (SQLITE_OK != ret) {
		sqlite_log("sqlite3_bind_int64", ret);
		goto error;
	}
	ret = sqlite3_bind_int(stmt, bind_index++, p->interval);
	if (SQLITE_OK != ret) {
		sqlite_log("sqlite3_bind_int", ret);
		goto error;
	}

	for (i = 0; i < columns_nel; i++) {
		double value;

		if (columns[i].get(&p->data, &value) == 0) {
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
		ret = sqlite3_step(stmt);
		if (ret != SQLITE_DONE) {
			sqlite_log("sqlite3_step", ret);
			goto error;
		}
	}

	return 0;

error:
	return -1;
}

int
sqlite_init(void)
{
	int ret;
	struct stat buf;
	int oflag = 0;
	const char* dbfile = confp->archive.sqlite.db;

	/* Clear */
	db = NULL;
	stmt = NULL;

	ret = sqlite3_initialize();
	if (ret != SQLITE_OK) {
		sqlite_log("sqlite3_initialize", ret);
		goto error;
	}

	/* Open database */
	ret = stat(dbfile, &buf);
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
		ret = sqlite3_exec(db, SQL_CREATE, NULL, NULL, NULL);
		if (ret != SQLITE_OK) {
			sqlite_log("sqlite3_exec", ret);
			goto error;
		}
	}

	/* Prepare statement */
	ret = sqlite3_prepare_v2(db, SQL_INSERT, -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		sqlite_log("sqlite3_prepare_v2", ret);
		goto error;
	}

	return 0;

error:
	if (db != NULL) {
		(void) sqlite3_close_v2(db);
		db = NULL;
	}
	return -1;
}

ssize_t
sqlite_insert(const struct ws_archive *p, size_t nel)
{
	size_t i;

	for (i = 0; i < nel; i++) {
		if (stmt_insert(&p[i]) == -1) {
			return -1;
		}
	}

	return nel;
}

static void
fetch_loop_columns(struct ws_loop *p, sqlite3_stmt *stmt, int col_index)
{
	int i;

	p->wl_mask = 0;

	for (i = 0; i < columns_nel; i++) {
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

	/* Prepare query */
	ret = sqlite3_prepare_v2(db, SQL_SELECT, -1, &query, NULL);
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

		p[i].data.time = sqlite3_column_int64(query, col_idx++);
		p[i].interval = sqlite3_column_int(query, col_idx++);

		fetch_loop_columns(&p->data, query, col_idx);

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
