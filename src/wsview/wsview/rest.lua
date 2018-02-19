local rest = { }

local util = require "luci.util"
local http = require "wsview.http"

local drv
local cnx

local function sql_archive(s, e)
	local sql = "SELECT time, "
	sql = sql .. "interval, "
	sql = sql .. "temp, "
	sql = sql .. "dew_point, "
	sql = sql .. "humidity, "
	sql = sql .. "wind_speed, "
	sql = sql .. "wind_dir, "
	sql = sql .. "hi_wind_speed, "
	sql = sql .. "barometer "
	sql = sql .. "FROM ws_archive "
	sql = sql .. string.format("WHERE %d <= time AND time < %d ", s, e)
	sql = sql .. "ORDER BY time"

	return cnx:execute(sql)
end

local function sql_month(s, e)
	local sql = "SELECT date(time, 'unixepoch') AS day, "
	sql = sql .. "MIN(lo_temp) AS lo_temp, "
	sql = sql .. "MAX(hi_temp) AS hi_temp, "
	sql = sql .. "SUM(rain_fall) AS rain_fall, "
	sql = sql .. "AVG(avg_wind_speed) AS avg_wind_speed, "
	sql = sql .. "MAX(hi_wind_speed) AS hi_wind_speed, "
	sql = sql .. "AVG(barometer) AS barometer "
	sql = sql .. "FROM ws_archive "
	sql = sql .. string.format("WHERE %d <= time AND time < %d ", s, e)
	sql = sql .. "GROUP BY date(time, 'unixepoch') "
	sql = sql .. "ORDER BY date(time, 'unixepoch')"

	return cnx:execute(sql)
end

local function sql_year(s, e)
	local sql = "SELECT substr(day, 1, 7) AS month, "
	sql = sql .. "AVG(lo_temp) AS lo_temp, "
	sql = sql .. "AVG(hi_temp) AS hi_temp, "
	sql = sql .. "SUM(rain_fall) AS rain, "
	sql = sql .. "MAX(rain_fall) AS rain_24h "
	sql = sql .. "FROM ( "
	sql = sql .. "SELECT date(time, 'unixepoch') AS day, "
	sql = sql .. "MIN(lo_temp) AS lo_temp, "
	sql = sql .. "MAX(hi_temp) AS hi_temp, "
	sql = sql .. "SUM(rain_fall) AS rain_fall "
	sql = sql .. "FROM ws_archive "
	sql = sql .. string.format("WHERE %d <= time AND time < %d ", s, e)
	sql = sql .. "GROUP BY date(time, 'unixepoch') "
	sql = sql .. ") q "
	sql = sql .. "GROUP BY substr(day, 1, 7) "
	sql = sql .. "ORDER BY substr(day, 1, 7)"

	return cnx:execute(sql)
end

local function write_json(n, obj)
	http.write(string.format('"%s":', n))
	http.write_json(obj)
end

local function rest_cb_year(row)
	local month_num = string.gsub(row.month, '.*-', '')

	row.month = tonumber(month_num)
end

local function rest_cb_month(row)
	local day_num = string.gsub(row.day, '.*-', '')

	row.day = tonumber(day_num)
end

local function rest_json(cur, cb)
	local first = true
	local row = cur:fetch({}, "a")

	http.write('"data":[')
	while row do
		if (not first) then
			http.write(",")
		end

		if cb ~= nil then
			cb(row)
		end
		http.write_json(row)

		-- next row
		first = false
		row = cur:fetch({}, "a")
	end
	http.write("]")

end

local function init(env, db)
	http.header("Content-Type", "application/json")

	if db and not cnx then
		local driver = require "luasql.sqlite3"

		drv = driver.sqlite3()
		cnx = drv:connect("/u12/wslogd.db")
	end
end

local function close(env)
	if cnx and not env.MOD_LUA then
		cnx:close()
		drv:close()
	end
end

local function rest_dump(cur, period, cb)
	http.write('{')
	write_json('period', period)
	http.write(',')
	rest_json(cur, cb)
	http.write('}')

	cur:close()
end

function rest.current(env)
	init(env)

	local wsview = require "wsview"

	http.write_json(wsview.current())
	close(env)
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

	local cur = sql_month(from, to)

	rest_dump(cur, { year = y, month = m }, rest_cb_month)
	close(env)
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

	local cur = sql_year(from, to)

	rest_dump(cur, { year = y }, rest_cb_year)
	close(env)
end

return rest

