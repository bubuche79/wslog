local rest = { }

local http = require "wsview.http"

local drv
local cnx

local function archive(s, e)
	local sql = "SELECT time, "
	sql = sql .. "barometer, "
	sql = sql .. "temp, "
	sql = sql .. "lo_temp, "
	sql = sql .. "hi_temp, "
	sql = sql .. "humidity, "
	sql = sql .. "rain_fall, "
	sql = sql .. "hi_rain_rate, "
	sql = sql .. "avg_wind_speed, "
	sql = sql .. "22.5*avg_wind_dir AS avg_wind_dir, "
	sql = sql .. "hi_wind_speed "
	sql = sql .. "FROM ws_archive "
	sql = sql .. string.format("WHERE %d <= time AND time < %d ", s, e)
	sql = sql .. "ORDER BY time"

	return cnx:execute(sql)
end

local function aggr_month(s, e)
	local sql = "SELECT date(time, 'unixepoch', 'localtime') AS time, "
	sql = sql .. "MIN(lo_temp) AS lo_temp, "
	sql = sql .. "MAX(hi_temp) AS hi_temp, "
	sql = sql .. "SUM(rain_fall) AS rain_fall, "
	sql = sql .. "AVG(avg_wind_speed) AS avg_wind_speed, "
	sql = sql .. "MAX(hi_wind_speed) AS hi_wind_speed, "
	sql = sql .. "AVG(barometer) AS barometer "
	sql = sql .. "FROM ws_archive "
	sql = sql .. string.format("WHERE %d <= time AND time < %d ", s, e)
	sql = sql .. "GROUP BY date(time, 'unixepoch', 'localtime') "
	sql = sql .. "ORDER BY date(time, 'unixepoch', 'localtime')"

	return cnx:execute(sql)
end

local function aggr_year(s, e)
	local sql = "SELECT substr(day, 1, 7) AS time, "
	sql = sql .. "AVG(lo_temp) AS lo_temp, "
	sql = sql .. "AVG(hi_temp) AS hi_temp, "
	sql = sql .. "SUM(rain_fall) AS rain, "
	sql = sql .. "MAX(rain_fall) AS rain_24h "
	sql = sql .. "FROM ( "
	sql = sql .. "SELECT date(time, 'unixepoch', 'localtime') AS day, "
	sql = sql .. "MIN(lo_temp) AS lo_temp, "
	sql = sql .. "MAX(hi_temp) AS hi_temp, "
	sql = sql .. "SUM(rain_fall) AS rain_fall "
	sql = sql .. "FROM ws_archive "
	sql = sql .. string.format("WHERE %d <= time AND time < %d ", s, e)
	sql = sql .. "GROUP BY date(time, 'unixepoch', 'localtime') "
	sql = sql .. ") q "
	sql = sql .. "GROUP BY substr(day, 1, 7) "
	sql = sql .. "ORDER BY substr(day, 1, 7)"

	return cnx:execute(sql)
end

local function init(env, db)
	http.content("application/json")

	db = false

	if db and not cnx then
		local driver = require "luasql.sqlite3"
		local util = require "wsview.util"

		local fname = util.getconf('archive.sqlite.db')

		drv = driver.sqlite3()
		cnx = drv:connect(fname)
	end
end

local function close(env)
	if cnx and not env.MOD_LUA then
		cnx:close()
		drv:close()
	end
--	if not env.MOD_LUA then
--		wsview.close()
--	end
end

local function dump_cur(cur)
	local first = true
	local row = cur:fetch({}, "a")

	http.write('[')
	while row do
		if (not first) then
			http.write(",")
		end

		http.write_json(row)

		-- next row
		first = false
		row = cur:fetch({}, "a")
	end
	http.write(']')

	cur:close()
end

function rest.current(env)
	init(env)

	local wsview = require "wsview"

	http.write_json(wsview.current())
	close(env)
end

function rest.archive(env)
	local y, m
	local t = os.date("*t")

	init(env, true)

	local from = os.time(t) - 4 * 24 * 3600
	local to = os.time(t)

	if cnx then
		local cur = archive(from, to)

		dump_cur(cur)
		close(env)
	else
		local wsview = require "wsview"
		local data = wsview.archive(from, to)

		http.write_json(data)
	end
end

function rest.month(env)
	local y, m
	local t = os.date("*t")

	init(env, true)

	if not env.ARGS then
		y = t.year
		m = t.month
	else
		y = tonumber(env.ARGS[1])
		m = tonumber(env.ARGS[2])

		if t.year < y or (t.year == y and t.month < m) then
			http.status(400)
			return
		end
	end

	local from = os.time({ year = y, month = m, day = 1, hour = 0 })
	local to = os.time({ year = y, month = m+1, day = 1, hour = 0 })

	if cnx then
		local cur = aggr_month(from, to)

		dump_cur(cur)
		close(env)
	else
		local wsview = require "wsview"
		local data = wsview.aggregate("month", from, to)

		http.write_json(data)
	end
end

function rest.year(env)
	local y
	local t = os.date("*t")

	init(env, true)

	if not env.ARGS then
		y = t.year
	else
		y = tonumber(env.ARGS[1])

		if t.year < y then
			http.status(400)
			return
		end
	end

	local from = os.time({ year = y, month = 1, day = 1, hour = 0 })
	local to = os.time({ year = y+1, month = 1, day = 1, hour = 0 })

	if cnx then
		local cur = aggr_year(from, to)

		dump_cur(cur)
		close(env)
	else
		local wsview = require "wsview"
		local data = wsview.aggregate("year", from, to)

		http.write_json(data)
	end
end

return rest

