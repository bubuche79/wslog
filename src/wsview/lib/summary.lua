require "wslog.util"

function print_value(v, metric, unit)
	print(string.format("<td class='data-%s'>", metric))
	if (v == nil) then
		print('--')
	else
		print(string.format("%.1f <span>%s</span>", v, unit))
	end
	print("</td>")
end

function print_line(metric, unit, row)
	print("<tr>")
	print(string.format("<td>%s</td>", translate(metric)))
	print_value(row[metric .. "_min"], metric, unit)
	print_value(row[metric .. "_max"], metric, unit)
	print_value(row[metric .. "_avg"], metric, unit)
	print("</tr>")
end

function display_date(s, e)
	local fields = { "temp", "dew_point", "humidity", "rain", "wind_speed", "wind_gust", "wind_dir", "barometer" }

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

	print("<div id='summary'>")
	print("<h2 class='summary'>Summary</h2>")

	local table_start = [[<table class='weather-summary'>
<thead>
<tr>
<td></td>
<td>Low</td>
<td>High</td>
<td>Average</td>
</tr>
</thead>]]

	local table_end = [[</table>]]

	print(table_start)
	print_line("temp", "°C", row)
	print_line("dew_point", "°C", row)
	print_line("humidity", "%", row)
	print_line("rain", "mm", row)
	print(table_end)

	print(table_start)
	print_line("wind_speed", "m/s", row)
	print_line("wind_gust", "m/s", row)
	print_line("wind_dir", "°", row)
	print_line("barometer", "hPa", row)
	print(table_end)

	print("</div>")
end

function display()
	local s = os.time{year=2016, month=12, day=1}
	local e = os.time{year=2017, month=12, day=1}

	driver = require "luasql.sqlite3"
	env = driver.sqlite3()
	cnx = env:connect("/u12/wslogd.db")

	display_date(s, e)

	cnx:close()
	env:close()
end
