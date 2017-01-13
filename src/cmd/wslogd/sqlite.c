#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <syslog.h>
#include <sqlite3.h>

#include "board.h"
#include "wslogd.h"
#include "sqlite.h"

#define try_sqlite(fn, ...) \
	do { \
		int _ret; \
		_ret = fn(__VA_ARGS__); \
		if (SQLITE_OK != _ret) { \
			syslog(LOG_ERR, "%s: %s", #fn, sqlite3_errstr(_ret)); \
			goto error; \
		} \
	} while (0)

#define try_sqlite_bind(fn, stmt, index, null, value) \
	do { \
		int _ret; \
		sqlite3_stmt *_stmt = (stmt); \
		int _index = (index); \
		int _null = (null); \
		__typeof__(value) _value = (value); \
		if (_null) { \
			_ret = fn(_stmt, _index, _value); \
		} else { \
			_ret = sqlite3_bind_null(_stmt, _index); \
		} \
		if (SQLITE_OK != _ret) { \
			syslog(LOG_ERR, "%s: %s", #fn, sqlite3_errstr(_ret)); \
			goto error; \
		} \
	} while (0)

#define try_sqlite_bind_int(stmt, index, null, value) \
	try_sqlite_bind(sqlite3_bind_int, (stmt), (index), (null), (value))

#define try_sqlite_bind_int64(stmt, index, null, value) \
	try_sqlite_bind(sqlite3_bind_int64, (stmt), (index), (null), (value))

#define try_sqlite_bind_double(stmt, index, null, value) \
	try_sqlite_bind(sqlite3_bind_double, (stmt), (index), (null), (value))

#define CREATE_TABLE \
	"CREATE TABLE ws_log ( " \
	"  tstamp INTEGER NOT NULL, " \
	"  wind_dir INTEGER, " \
	"  wind_speed REAL, " \
	"  humidity INTEGER, " \
	"  dew_point REAL, " \
	"  temp REAL, " \
	"  rain_1 REAL, " \
	"  rain_24 REAL, " \
	"  temp_in REAL, " \
	"  humidity_in INTEGER, " \
	"  PRIMARY KEY (tstamp)" \
	") "

#define INSERT_INTO \
	"INSERT INTO ws_log " \
	"(tstamp, wind_dir, wind_speed, humidity, dew_point, temp, rain_1, rain_24, temp_in, humidity_in) " \
	"VALUES " \
	"(?, ?, ?, ?, ?, ?, ?, ?, ?, ?) "

static sqlite3 *db;
static sqlite3_stmt *stmt;

int
sqlite_init(void)
{
	int ret;
	struct stat buf;
	int oflag = 0;
	const char* dbfile = confp->sqlite.db;

	/* Clear */
	db = NULL;
	stmt = NULL;

	try_sqlite(sqlite3_initialize);

	/* Open database */
	ret = stat(dbfile, &buf);
	if (ret == -1) {
		if (errno == ENOENT) {
			oflag |= SQLITE_OPEN_CREATE;
		} else {
			syslog(LOG_ERR, "stat(%s): %m", dbfile);
			goto error;
		}
	}

	oflag |= SQLITE_OPEN_READWRITE;

	ret = sqlite3_open_v2(dbfile, &db, oflag, NULL);
	if (ret != SQLITE_OK) {
		syslog(LOG_ERR, "sqlite3_open_v2(%s): %s", dbfile, sqlite3_errstr(ret));
		goto error;
	}

	/* Create schema */
	if (oflag & SQLITE_OPEN_CREATE) {
		try_sqlite(sqlite3_exec, db, CREATE_TABLE, NULL, NULL, NULL);
	}

	/* Prepare statement */
	try_sqlite(sqlite3_prepare_v2, db, INSERT_INTO, -1, &stmt, NULL);

	return 0;

error:
	if (db != NULL) {
		(void) sqlite3_close_v2(db);
		db = NULL;
	}
	return -1;
}

int
sqlite_write(void)
{
	int ret;
	int bind_index;
	struct ws_log ws;

	board_get(&ws);

	try_sqlite(sqlite3_reset, stmt);

	/* Bind variables */
	bind_index = 1;
	try_sqlite_bind_int64(stmt, bind_index++, 0, ws.time);
	try_sqlite_bind_int(stmt, bind_index++, ws_isset(&ws, WF_WIND_DIR), ws.wind_dir);
	try_sqlite_bind_double(stmt, bind_index++, ws_isset(&ws, WF_WIND_SPEED), ws.wind_speed);
	try_sqlite_bind_int(stmt, bind_index++, ws_isset(&ws, WF_HUMIDITY), ws.humidity);
	try_sqlite_bind_double(stmt, bind_index++, ws_isset(&ws, WF_DEW_POINT), ws.dew_point);
	try_sqlite_bind_double(stmt, bind_index++, ws_isset(&ws, WF_TEMP), ws.temp);
	try_sqlite_bind_double(stmt, bind_index++, ws_isset(&ws, WF_RAIN_1H), ws.rain_1h);
	try_sqlite_bind_double(stmt, bind_index++, ws_isset(&ws, WF_RAIN_24H), ws.rain_24h);
	try_sqlite_bind_double(stmt, bind_index++, ws_isset(&ws, WF_TEMP_IN), ws.temp_in);
	try_sqlite_bind_int(stmt, bind_index++, ws_isset(&ws, WF_HUMIDITY_IN), ws.humidity_in);

	/* Execute */
	ret = sqlite3_step(stmt);
	if (ret != SQLITE_DONE) {
		syslog(LOG_ERR, "sqlite3_step: %s", sqlite3_errstr(ret));
		goto error;
	}

	return 0;

error:
	return -1;
}

int
sqlite_destroy(void)
{
	int ret;
	int exit;

	exit = 0;

	ret = sqlite3_finalize(stmt);
	if (ret != SQLITE_OK) {
		exit = -1;
		syslog(LOG_ERR, "sqlite3_finalize: %s", sqlite3_errstr(ret));
	}

	if (db != NULL) {
		ret = sqlite3_close_v2(db);
		if (ret != SQLITE_OK) {
			exit = -1;
			syslog(LOG_ERR, "sqlite3_close_v2: %s", sqlite3_errstr(ret));
		}
	}

	ret = sqlite3_shutdown();
	if (ret != SQLITE_OK) {
		exit = -1;
		syslog(LOG_ERR, "sqlite3_shutdown: %s", sqlite3_errstr(ret));
	}

	return exit;
}
