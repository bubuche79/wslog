local archive = { }

local http = require "wsview.http"
local template = require "wsview.template"
local graphics = require "wsview.graphics"

local function archive_charts(p)
	local defs = { }

	defs[1] = { name = "temp" }
	defs[2] = { name = "wind" }
	defs[3] = { name = "pressure" }

	graphics.charts("archive", defs, p)
end

local function archive_table(p)
	graphics.table({ name = "archive" }, p)
end

local function main(env, p)
	local t = os.date("*t")
	local prefix = "/cgi-bin/wsview/archive"

	http.write([[<ul class="tabs">]])
	graphics.tab("charts", prefix, p.view)
	graphics.tab("table", prefix, p.view)

	http.write([[</ul>]])

	if next(env.ARGS) then
		p.year = tonumber(env.ARGS[1])
		p.month = tonumber(env.ARGS[2])
		p.day = tonumber(env.ARGS[3])
	end

	if (p.view == "table") then
		archive_table(p)
	else
		archive_charts(p)
	end
end

function archive.charts(env, params)
	template.header()
	params.view = "chart"
	main(env, params)
	template.footer()
end

function archive.table(env, params)
	template.header()
	params.view = "table"
	main(env, params)
	template.footer()
end

function archive.index(env, params)
	archive.charts(env, params)
end

return archive

