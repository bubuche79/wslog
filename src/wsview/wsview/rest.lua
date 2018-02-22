local rest = { }

local util = require "luci.util"
local wsview = require "wsview"
local http = require "wsview.http"

local function init(env)
	http.content("application/json")
end

local function close(env)
	if not env.MOD_LUA then
		wsview.close()
	end
end

function rest.current(env)
	init()

	http.write_json(wsview.current())
	close(env)
end

function rest.month(env)
	local y, m
	local t = os.date("*t")

	init()

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

	local data = wsview.aggregate("month", from, to)

	http.write_json(data)
	close(env)
end

function rest.year(env)
	local y
	local t = os.date("*t")

	init()

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

	local data = wsview.aggregate("year", from, to)

	http.write_json(data)
	close(env)
end

return rest

