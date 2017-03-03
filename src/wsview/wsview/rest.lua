local coroutine = require "coroutine"
local http = require "wsview.http"

function sql_query(s, e)
	local sql = "SELECT date(time, 'unixepoch') AS day, "
	sql = sql .. "MIN(temp) AS temp_min, "
	sql = sql .. "MAX(temp) AS temp_max, "
	sql = sql .. "SUM(rain) AS rain, "
	sql = sql .. "AVG(wind_speed) AS wind_speed, "
	sql = sql .. "MAX(wind_gust_speed) AS wind_gust_speed "
	sql = sql .. "FROM ws_archive "
	sql = sql .. string.format("WHERE %d <= time AND time < %d ", s, e)
	sql = sql .. "GROUP BY date(time, 'unixepoch') "
	sql = sql .. "ORDER BY date(time, 'unixepoch') "

	return cnx:execute(sql)
end

function write_json(n, obj)
	http.write(string.format('"%s":', n))
	http.write_json(obj)
end

function rest_json(s, e)
	local first = true

	local cur = sql_query(s, e)
	local row = cur:fetch({}, "a")

	http.write(',"data":[')
	while row do
		if (not first) then
			http.write(",")
		end

		local day_num = string.gsub(row.day, '.*-', '')

		row.day = tonumber(day_num)
		http.write_json(row)

		-- next row
		first = false
		row = cur:fetch({}, "a")
	end
	http.write("]")

	cur:close()
end

function aggr_month(y, m)
	local from = os.time({ year = y, month = m, day = 1 })
	local to = os.time({ year = y, month = m+1, day = 1 })

	http.prepare_content("application/json; charset=utf-8")

	http.write('{')
	write_json('period', { year = y, month = m })

	rest_json(from, to)

	http.write('}')
	http.close()
end
