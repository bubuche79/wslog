local coroutine = require "coroutine"
local http = require "wsview.http"

local function rest_json(s, e)
	local first = true
	local sql = "SELECT * FROM ws_archive "
	sql = sql .. string.format("WHERE %d < time AND time < %d", s, e)

	local cur = cnx:execute(sql)
	local row = cur:fetch({}, "a")

	http.write("{")
	while row do
		if (not first) then
			http.write(",")
		end

		http.write("[")
		http.write_json(row)
		http.write("]")

		-- next row
		first = false
		row = cur:fetch({}, "a")
	end
	http.write("}")
	http.close()

	cur:close()
end

function rest_today()
	local now = os.time()
	local e = os.date("*t", now)
	local s = os.time({ year = e.year, month = e.month, day = e.day, hour = 0, isdst = e.isdst })

	--print("<span class='date'>" .. i18n_date(s) .. "</span>")

	driver = require "luasql.sqlite3"
	env = driver.sqlite3()
	cnx = env:connect("/var/run/wslogd.db")

	http.prepare_content("application/json; charset=utf-8")

	rest_json(s, now)

	cnx:close()
	env:close()
end

