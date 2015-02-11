local rest = {}

local http = require "wsview.http"
local wsview = require "wsview"

function rest.current( env)
	local dat = wsview.current()

	wsview.close()
	http.content("application/json")
	http.write_json(dat)
end

function rest.archive(env)
	local from, to

	if next(env.ARGS) == nil then
		local t = os.date("*t")

		to = os.time(t)
		from = to - 4 * 24 * 3600
	else
		local y, m, d

		y = tonumber(env.ARGS[1])
		m = tonumber(env.ARGS[2])
		d = tonumber(env.ARGS[3])

		from = os.time({ year = y, month = m, day = d, hour = 0 })
		to = os.time({ year = y, month = m, day = d + 1, hour = 0 })
	end

	local dat = wsview.archive(from, to)

	http.content("application/json")
	http.write_json({data = dat, from = from, to = to})
	wsview.close()
end

return rest
