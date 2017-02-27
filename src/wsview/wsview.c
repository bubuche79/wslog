#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <errno.h>

#include "defs/dso.h"
#include "defs/std.h"
#include "libws/util.h"

#include "board.h"
#include "dataset.h"
#include "wsview.h"

struct lua_table
{
	const char *name;
	int (*get) (const struct ws_loop *, double *);
};

static int
get_current(lua_State *L)
{
	int res;
	size_t i, nel;
	struct ws_loop buf;
	struct ws_loop *bufp;

	struct lua_table fields[] =
	{
		{ "pressure", ws_get_pressure },
		{ "barometer", ws_get_barometer },
		{ "temp", ws_get_temp },
		{ "humidity", ws_get_humidity },
		{ "wind_speed", ws_get_wind_speed },
		{ "wind_dir", ws_get_wind_dir },
		{ "wind_gust_speed", ws_get_wind_gust_speed },
		{ "wind_gust_dir", ws_get_wind_gust_dir },
		{ "rain", ws_get_rain },
		{ "rain_rate", ws_get_rain_rate },
		{ "dew_point", ws_get_dew_point },
		{ "windchill", ws_get_windchill },
		{ "heat_index", ws_get_heat_index },
		{ "temp_in", ws_get_temp_in },
		{ "humidity_in", ws_get_humidity_in }
	};

	/* Read shared board */
	if (board_lock() == -1) {
		goto error;
	}

	bufp = board_peek(0);

	if (bufp != NULL) {
		memcpy(&buf, bufp, sizeof(buf));
	}

	if (board_unlock() == -1) {
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

		/* Add time */
		lua_pushinteger(L, bufp->time);
		lua_setfield(L, -2, "time");
	}

	return res;

error:
	lua_pushfstring(L, "board_unlock: %s", strerror(errno));
	return lua_error(L);
}

static int
get_wind_dir(lua_State *L)
{
	double dir;
	const char *res;

	dir = lua_tonumber(L, 1);
	res = ws_dir(dir);

	lua_pushstring(L, res);

	return 1;
}

DSO_EXPORT int
luaopen_wsview(lua_State *L)
{
	lua_register(L, "ws_current", get_current);
	lua_register(L, "ws_wind_dir", get_wind_dir);

	if (board_open(0) == -1) {
        lua_pushfstring(L, "board_open: %s", strerror(errno));
		return lua_error(L);
	}

    return 0;
}
