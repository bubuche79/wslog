local coroutine = require "coroutine"
local http = require "wsview.http"

function sql_query(s, e)
	local sql = "SELECT date(time, 'unixepoch') AS day, "
	sql = sql .. "MIN(temp) AS temp_min, "
	sql = sql .. "MAX(temp) AS temp_max, "
	sql = sql .. "SUM(rain) AS rain "
	sql = sql .. "FROM ws_archive "
	sql = sql .. string.format("WHERE %d < time AND time < %d ", s, e)
	sql = sql .. "GROUP BY date(time, 'unixepoch') "
	sql = sql .. "ORDER BY date(time, 'unixepoch') "

	return cnx:execute(sql)
end

function rest_json(s, e)
	local first = true

	local cur = sql_query(s, e)
	local row = cur:fetch({}, "a")

	local now = os.date("*t", e)

	http.write('{"period":')
	http.write_json({ year = now.year, month = now.month });
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
	http.write("]}")
	http.close()

	cur:close()
end

function rest_today()
	local now = os.time()
	local e = os.date("*t", now)
	local s = os.time({ year = e.year, month = e.month, day = 0, hour = 0, isdst = e.isdst })

	http.prepare_content("application/json; charset=utf-8")

	rest_json(s, now)
end

