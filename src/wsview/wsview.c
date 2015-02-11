#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <lauxlib.h>
#include <sqlite3.h>

#include "libws/defs.h"
#include "libws/util.h"

#include "board.h"
#include "dataset.h"
#include "conf.h"
#include "wsview.h"

static int board = 0;
static sqlite3 *db = NULL;

struct lua_table
{
	const char *name;
	int (*get) (const struct ws_loop *, double *);
};

static void
db_open(const char *path)
{
	sqlite3_initialize();
	sqlite3_open_v2(path, &db, SQLITE_OPEN_READONLY, NULL);
}

static void
lua_push_int(lua_State *L, sqlite3_stmt* stmt, int idx, const char *name)
{
	long long v;

	v = sqlite3_column_int64(stmt, idx);
	lua_pushinteger(L, v);
	lua_setfield(L, -2, name);
}

static void
lua_push_double(lua_State *L, sqlite3_stmt* stmt, int idx, const char *name)
{
	double v;

	v = sqlite3_column_double(stmt, idx);
	lua_pushnumber(L, v);
	lua_setfield(L, -2, name);
}

static void
lua_push_text(lua_State *L, sqlite3_stmt* stmt, int idx, const char *name)
{
	const char *v;

	v = (char *) sqlite3_column_text(stmt, idx);
	lua_pushstring(L, v);
	lua_setfield(L, -2, name);
}

static void
lua_load_stmt(lua_State *L, sqlite3_stmt *stmt)
{
	int rows, n;

	n = 1;
	rows = sqlite3_column_count(stmt);

	lua_newtable(L);

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		int i;

		lua_pushinteger(L, n++);
		lua_newtable(L);

		for (i = 0; i < rows; i++) {
			int type;
			const char *name;

			type = sqlite3_column_type(stmt, i);
			name = sqlite3_column_name(stmt, i);

			switch (type) {
			case SQLITE_INTEGER:
				lua_push_int(L, stmt, i, name);
				break;
			case SQLITE_FLOAT:
				lua_push_double(L, stmt, i, name);
				break;
			case SQLITE_TEXT:
				lua_push_text(L, stmt, i, name);
				break;
			default:
				break;
			}
		}

		lua_settable(L, -3);
	}
}

static int
wsview_query(lua_State *L, const char *sql, time_t lower, time_t upper)
{
	sqlite3_stmt *stmt;

	/* Open database */
	if (db == NULL) {
		const char *path = getenv("WSLOG_SQLITE3");

		if (path == NULL) {
			path = WS_CONF_SQLITE_DB;
		}
		db_open(path);
	}

	sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	sqlite3_bind_int64(stmt, 1, lower);
	sqlite3_bind_int64(stmt, 2, upper);

	lua_load_stmt(L, stmt);

	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);

	return 1;
}

static int
wsview_archive(lua_State *L)
{
	time_t lower = lua_tonumber(L, 1);
	time_t upper = lua_tonumber(L, 2);

	const char sql[] =
		"SELECT time, "
		  "barometer, "
		  "temp, "
		  "humidity, "
		  "avg_wind_speed, "
		  "avg_wind_dir, "
		  "hi_wind_speed, "
		  "rain_fall, "
		  "hi_rain_rate, "
		  "dew_point, "
		  "windchill, "
		  "heat_index "
		"FROM ws_archive "
		"WHERE ? < time AND time <= ? "
		"ORDER BY time";

	return wsview_query(L, sql, lower, upper);
}

static int
wsview_aggr_day(lua_State *L, time_t lower, time_t upper)
{
	const char sql[] =
		"SELECT date(time - 1, 'unixepoch', 'localtime') AS time, "
		  "MIN(lo_temp) AS lo_temp, "
		  "MAX(hi_temp) AS hi_temp, "
		  "SUM(rain_fall) AS rain_fall, "
		  "AVG(avg_wind_speed) AS avg_wind_speed, "
		  "MAX(hi_wind_speed) AS hi_wind_speed, "
		  "AVG(barometer) AS barometer "
		"FROM ws_archive "
		"WHERE ? < time AND time <= ? "
		"GROUP BY date(time - 1, 'unixepoch', 'localtime') "
		"ORDER BY date(time - 1, 'unixepoch', 'localtime')";

	return wsview_query(L, sql, lower, upper);
}

static int
wsview_aggr_month(lua_State *L, time_t lower, time_t upper)
{
	const char sql[] =
		"SELECT substr(day, 1, 7) AS time, "
		  "AVG(lo_temp) AS lo_temp, "
		  "AVG(hi_temp) AS hi_temp, "
		  "SUM(rain_fall) AS rain, "
		  "MAX(rain_fall) AS rain_24h "
		"FROM ( "
		  "SELECT date(time - 1, 'unixepoch', 'localtime') AS day, "
		    "MIN(lo_temp) AS lo_temp, "
		    "MAX(hi_temp) AS hi_temp, "
		    "SUM(rain_fall) AS rain_fall "
		  "FROM ws_archive "
		  "WHERE ? < time AND time <= ? "
		  "GROUP BY date(time - 1, 'unixepoch', 'localtime') "
		") q "
		"GROUP BY substr(day, 1, 7) "
		"ORDER BY substr(day, 1, 7)";

	return wsview_query(L, sql, lower, upper);
}

static int
wsview_aggregate(lua_State *L)
{
	int ret;
	const char *method = lua_tostring(L, 1);
	time_t lower = lua_tonumber(L, 2);
	time_t upper = lua_tonumber(L, 3);

	if (!strcmp("day", method)) {
		ret = wsview_aggr_day(L, lower, upper);
	} else if (!strcmp("month", method)) {
		ret = wsview_aggr_month(L, lower, upper);
	} else {
		ret = 0;
	}

	return ret;
}

static int
wsview_current(lua_State *L)
{
	int res;
	size_t i, nel;
	struct ws_loop buf;
	struct ws_loop *bufp;

	struct lua_table fields[] =
	{
		{ "barometer", ws_loop_barometer },
		{ "temp", ws_loop_temp },
		{ "humidity", ws_loop_humidity },
		{ "wind_speed", ws_loop_wind_speed },
		{ "wind_dir", ws_loop_wind_dir },
		{ "rain_day", ws_loop_rain_day },
		{ "rain_rate", ws_loop_rain_rate },
		{ "solar_rad", ws_loop_solar_rad },
		{ "uv", ws_loop_uv },
		{ "dew_point", ws_loop_dew_point },
		{ "windchill", ws_loop_windchill },
		{ "heat_index", ws_loop_heat_index },
		{ "in_temp", ws_loop_in_temp },
		{ "in_humidity", ws_loop_in_humidity }
	};

	/* Read shared board */
	if (!board) {
		if (board_open(0) == -1) {
			lua_pushfstring(L, "board_open: %s", strerror(errno));
			goto error;
		}

		board = 1;
	}

	if (board_lock() == -1) {
		lua_pushfstring(L, "board_lock: %s", strerror(errno));
		goto error;
	}

	bufp = board_peek(0);

	if (bufp != NULL) {
		memcpy(&buf, bufp, sizeof(buf));
	}

	if (board_unlock() == -1) {
		lua_pushfstring(L, "board_unlock: %s", strerror(errno));
		goto error;
	}

	/* Lua result */
	if (bufp == NULL) {
		res = 0;
	} else {
		res = 1;

		lua_newtable(L);

		/* Add fields to table */
		nel = array_size(fields);

		for (i = 0; i < nel; i++) {
			double value;

			if (fields[i].get(bufp, &value) == 0) {
				lua_pushnumber(L, round_scale(value, 2));
				lua_setfield(L, -2, fields[i].name);
			}
		}

		lua_pushinteger(L, bufp->time.tv_sec);
		lua_setfield(L, -2, "time");
	}

	return res;

error:
	return lua_error(L);
}

static int
wsview_wind_dir(lua_State *L)
{
	double dir;
	const char *res;

	dir = lua_tonumber(L, 1);
	res = ws_dir_deg(dir);

	lua_pushstring(L, res);

	return 1;
}

static int
wsview_open(lua_State *L)
{
	const char *path = lua_tostring(L, 1);

	db_open(path);

	return 0;
}

static int
wsview_close(lua_State *L)
{
	if (board) {
		board_unlink();
		board = 0;
	}

	if (db) {
		sqlite3_close_v2(db);
		sqlite3_shutdown();
		db = NULL;
	}

	return 0;
}

int
luaopen_wsview(lua_State *L)
{
	luaL_Reg wslib[] =
	{
		{ "current", wsview_current },
		{ "wind_dir", wsview_wind_dir },
		{ "aggregate", wsview_aggregate },
		{ "archive", wsview_archive },
		{ "open", wsview_open },
		{ "close", wsview_close },
		{ NULL, NULL }
	};

	lua_newtable(L);
#if LUA_VERSION_NUM < 502
	luaL_register(L, NULL, wslib);
#else
	luaL_setfuncs(L, wslib, 0);
#endif

	return 1;
}
