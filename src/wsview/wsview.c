#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <errno.h>
#include <lauxlib.h>

#include "defs/dso.h"
#include "defs/std.h"
#include "libws/util.h"

#include "board.h"
#include "dataset.h"
#include "wsview.h"

static int board = 0;

struct lua_table
{
	const char *name;
	int (*get) (const struct ws_loop *, double *);
};

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

		lua_pushinteger(L, bufp->time);
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

static const luaL_Reg wslib[] =
{
	{ "current", wsview_current },
	{ "wind_dir", wsview_wind_dir },
	{ NULL, NULL }
};

DSO_EXPORT int
luaopen_wsview(lua_State *L)
{
	lua_newtable(L);
	luaL_register(L, NULL, wslib);

	return 1;
}
