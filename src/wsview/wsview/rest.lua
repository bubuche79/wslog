local coroutine = require "coroutine"
local http = require "wsview.http"

function sql_month(s, e)
	local sql = "SELECT date(time, 'unixepoch') AS day, "
	sql = sql .. "MIN(temp) AS temp_min, "
	sql = sql .. "MAX(temp) AS temp_max, "
	sql = sql .. "SUM(rain) AS rain, "
	sql = sql .. "AVG(wind_speed) AS wind_speed, "
	sql = sql .. "MAX(wind_gust_speed) AS wind_gust_speed, "
	sql = sql .. "AVG(barometer) AS barometer "
	sql = sql .. "FROM ws_archive "
	sql = sql .. string.format("WHERE %d <= time AND time < %d ", s, e)
	sql = sql .. "GROUP BY date(time, 'unixepoch') "
	sql = sql .. "ORDER BY date(time, 'unixepoch') "

	return cnx:execute(sql)
end

function sql_year(s, e)
	local sql = "SELECT substr(day, 1, 7) AS month, "
	sql = sql .. "AVG(temp_min) AS temp_min, "
	sql = sql .. "AVG(temp_max) AS temp_max, "
	sql = sql .. "SUM(rain) AS rain, "
	sql = sql .. "MAX(rain) AS rain_24h "
	sql = sql .. "FROM ( "
	sql = sql .. "SELECT date(time, 'unixepoch') AS day, "
	sql = sql .. "MIN(temp) AS temp_min, "
	sql = sql .. "MAX(temp) AS temp_max, "
	sql = sql .. "SUM(rain) AS rain "
	sql = sql .. "FROM ws_archive "
	sql = sql .. string.format("WHERE %d <= time AND time < %d ", s, e)
	sql = sql .. "GROUP BY date(time, 'unixepoch') "
	sql = sql .. ") q "
	sql = sql .. "GROUP BY substr(day, 1, 7) "
	sql = sql .. "ORDER BY substr(day, 1, 7) "

	return cnx:execute(sql)
end

function write_json(n, obj)
	http.write(string.format('"%s":', n))
	http.write_json(obj)
end

function rest_cb_year(row)
	local month_num = string.gsub(row.month, '.*-', '')

	row.month = tonumber(month_num)
end

function rest_cb_month(row)
	local day_num = string.gsub(row.day, '.*-', '')

	row.day = tonumber(day_num)
end

function rest_json(cur, cb)
	local first = true
	local row = cur:fetch({}, "a")

	http.write(',"data":[')
	while row do
		if (not first) then
			http.write(",")
		end

		cb(row)
		http.write_json(row)

		-- next row
		first = false
		row = cur:fetch({}, "a")
	end
	http.write("]")

end

function aggr_month(y, m)
	local from, to
	local cur

	if (m == nil) then
		from = os.time({ year = y, month = 1, day = 1, hour = 0 })
		to = os.time({ year = y+1, month = 1, day = 1, hour = 0 })
		cur = sql_year(from, to)
	else
		from = os.time({ year = y, month = m, day = 1, hour = 0 })
		to = os.time({ year = y, month = m+1, day = 1, hour = 0 })
		cur = sql_month(from, to)
	end

	http.prepare_content("application/json; charset=utf-8")
	http.write('{')

	if (m == nil) then
		write_json('period', { year = y })
		rest_json(cur, rest_cb_year)
	else
		write_json('period', { year = y, month = m })
		rest_json(cur, rest_cb_month)
	end

	http.write('}')
	http.close()

	cur:close()
end
