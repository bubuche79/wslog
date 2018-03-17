local archive = { }

local http = require "wsview.http"
local template = require "wsview.template"
local i18n = require "wsview.i18n"
local charts = require "wsview.charts"

local function archive_charts(p)
	local defs = { }

	defs[1] = { name = "temp" }
	defs[2] = { name = "wind" }

	charts.add("archive", defs, p)
end

local function main(env, p)
	local t = os.date("*t")
	local prefix = "/cgi-bin/wsview/archive"

	p.year = tonumber(p.year) or t.year
	p.month = tonumber(p.month) or t.month

	archive_charts(p)
end

function archive.index(env, params)
	template.header()
	main(env, params)
	template.footer()
end


return archive

