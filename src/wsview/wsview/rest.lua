local rest = { }

local http = require "wsview.http"
local wsview = require "wsview"

local function init(env)
	http.content("application/json")
end

local function close(env)
	if not env.MOD_LUA then
		wsview.close()
	end
end

local function aggr(unit, from, to)
	local res = { }

	res.unit = unit
	res.from = from	
	res.to = to
	res.data = wsview.aggregate(unit, from, to)

	http.write_json(res)
end

function rest.current(env)
	init(env)

	http.write_json(wsview.current())
	close(env)
end

function rest.archive(env)
	local res = {}

	init(env)

	if next(env.ARGS) == nil then
		local t = os.date("*t")

		res.to = os.time(t)
		res.from = res.to - 4 * 24 * 3600
	else
		local y, m, d

		y = tonumber(env.ARGS[1])
		m = tonumber(env.ARGS[2])
		d = tonumber(env.ARGS[3])

		res.from = os.time({ year = y, month = m, day = d, hour = 0 })
		res.to = os.time({ year = y, month = m, day = d+1, hour = 0 })
	end

	res.data = wsview.archive(res.from, res.to)

	http.write_json(res)
	close(env)
end

function rest.day(env)
	local y, m
	local t = os.date("*t")

	init(env)

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

	aggr("day", from, to)
	close(env)
end

function rest.month(env)
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

	aggr("month", from, to)
	close(env)
end

return rest

