require "wsview"
require "wsview.util"

function aggr_row(metric, unit, row, cols)
	local id = string.gsub(metric, "_", "-")
	local fmt = conv[unit]

	print("<div id='aggr-" .. id .. "' class='row'>")

	print("<div class='col'>")
	print("<span class='aggr-label'>" .. i18n_fmt(metric) .. "</span>")
	print("</div>")

	for i, m in pairs(cols) do
		local v = row[metric .. "_" .. m]

		print("<div class='col'>")
		print("<span class='aggr-data'>")
		if (v ~= nil) then
			printf("<span class='aggr-value'>" .. fmt.fmt .. "</span>", v)
			if (fmt.unit ~= nil) then
				printf("<span class='aggr-unit'>%s</span>", fmt.unit)
			end
		else
			print("<span class='aggr-value'>--</span>")
		end
		printf("</span></div>")
	end

	print("</div>")
end

function summary(s, e)
	local fields = { "temp", "dew_point", "humidity", "rain", "wind_speed", "wind_gust_speed", "wind_dir", "barometer" }

	local sql = "SELECT "
	for i, v in pairs(fields) do
		sql = sql .. string.format("MIN(%s) AS %s_min, ", v, v)
		sql = sql .. string.format("MAX(%s) AS %s_max, ", v, v)
		sql = sql .. string.format("AVG(%s) AS %s_avg, ", v, v)
	end
	sql = sql .. "NULL "
	sql = sql .. "FROM ws_archive "
	sql = sql .. string.format("WHERE %d < time AND time < %d", s, e)

	local cur = cnx:execute(sql)
	local row = cur:fetch({}, "a")
	cur:close()

	row.wind_dir_avg = ws_wind_dir(row.wind_dir_avg)

	-- print html
	print("<div id='aggr'>")
	print("<h2 class='summary'>" .. i18n_fmt('summary') .. "</h2>")

	print("<div class='table'>")
	aggr_row("temp", "temp", row, { "min", "max", "avg" })
	aggr_row("dew_point", "temp", row, { "min", "max", "avg" })
	aggr_row("humidity", "humidity", row, { "min", "max", "avg" })
	aggr_row("rain", "rain_rate", row, { "", "", "max" })
	aggr_row("wind_speed", "speed", row, { "min", "max", "avg" })
	aggr_row("wind_gust", "speed", row, { "", "max", "" })
	aggr_row("wind_dir", "dir", row, { "", "", "avg" })
	aggr_row("barometer", "pressure", row, { "min", "max", "" })
	print("</div>")

	print("</div>")
end

function summary_today()
	local now = os.time()
	local e = os.date("*t", now)
	local s = os.time({ year = e.year, month = e.month, day = e.day, hour = 0, isdst = e.isdst })

	--print("<span class='date'>" .. i18n_date(s) .. "</span>")

	driver = require "luasql.sqlite3"
	env = driver.sqlite3()
	cnx = env:connect("/var/run/wslogd.db")

	summary(s, now)

	cnx:close()
	env:close()
end

