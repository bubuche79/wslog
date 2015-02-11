local aggr = {}

local http = require "wsview.http"
local wsview = require "wsview"

local function aggregate(unit, from, to)
	local dat = wsview.aggregate(unit, from, to)

	wsview.close()

	http.content("application/json")
	http.write_json({ data = dat, unit = unit, from = from, to = to })
end

function aggr.today(env)
	local t = os.date("*t")

	local from = os.time({ year = t.year, month = t.month, day = t.day, hour = 0 })
	local to = os.time({ year = t.year, month = t.month, day = t.day + 1, hour = 0 })

	aggregate("day", from, to)
end

function aggr.day(env)
	local y, m
	local t = os.date("*t")

	if next(env.ARGS) == nil then
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

	aggregate("day", from, to)
end

function aggr.month(env)
	local y
	local t = os.date("*t")

	if next(env.ARGS) == nil then
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

	aggregate("month", from, to)
end

return aggr
