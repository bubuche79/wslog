local home = { }

local wsview = require "wsview"
local template = require "wsview.template"
local i18n = require "wsview.i18n"

local function current()
	http.write("<h2 class='current'>" .. i18n.translate('current') .. "</h2>")
	http.write("<div id='current'>")

	http.write("</div>")
end

local function rt_row(id, v, unit, label, split, el)
	local fmt = i18n.unit(unit)

	el = el or "span"
	split = split or 1

	http.write("<div id='rt-" .. id .. "' class='row'>")

	http.write("<div class='col'>")
	if (label ~= nil) then
		http.write("<span class='ws-label'>" .. i18n.translate(label) .. "</span>")
		if (split == 1) then
			http.write("</div><div class='col'>")
		end
	end
	http.format("<%s class='ws-data'>", el)
	if (v ~= nil) then
		http.format("<%s class='ws-value'>" .. fmt.fmt .. "</%s>", el, v, el)
		if (fmt.unit ~= nil) then
			http.format("<%s class='ws-unit'>%s</%s>", el, fmt.unit, el)
		end
	else
		http.format("<%s class='ws-value'>--</%s>", el, el)
	end
	http.format("</%s></div>", el)

	http.write("</div>")
end

local function div_row(metric, unit, tbl, label, split, el)
	local id = string.gsub(metric, "_", "-")
	local v = tbl[metric]

	rt_row(id, v, unit, label, split, el)
end

local function curr_hrow(metric, unit, tbl, label)
	div_row(metric, unit, tbl, label, 0)
end

local function curr_row(metric, unit, tbl)
	div_row(metric, unit, tbl, metric)
end

local function current_header(tbl)
	local dir

	-- temp
	http.write("<div class='col'>")
	http.write("<div class='table'>")
	curr_hrow("temp", "temp", tbl)
	curr_hrow("windchill", "temp", tbl, "feels")
	http.write("</div>")
	http.write("</div>")

	-- wind
	http.write("<div class='col'>")
	http.write("<div class='table'>")

	http.write("<div id='rt-wind' class='row'>")
	http.write("<div class='col'>")
	if (tbl.wind_dir ~= nil) then
		dir = wsview.wind_dir(tbl.wind_dir)

		http.format("<div style='transform: rotate(%ddeg); '>", tbl.wind_dir)
		http.write("<div class='circle'><div class='arrow'></div></div>")
		http.write("</div>")
	else
		dir = "--"
	end
	div_row("wind_speed", "speed", tbl, nil, 0, "div")
	http.write("</div>")
	http.write("</div>")

	rt_row("wind-dir", dir, "dir", "wind_from", 0)

	http.write("</div>")
	http.write("</div>")
end

local function current_body(tbl)
	http.write("<div class='col'>")
	http.write("<div id='rt-tbody' class='table'>")
	curr_row("dew_point", "temp", tbl)
	curr_row("humidity", "humidity", tbl)
	curr_row("rain_rate", "rain_rate", tbl)
	curr_row("barometer", "pressure", tbl)
	http.write("</div>")
	http.write("</div>")
end

local function current()
	local tbl = wsview.current()

	http.write("<h2>" .. i18n.translate('current') .. "</h2>")
	http.write("<div id='current'>")

	if (tbl == nil) then
		http.write("The wslogd daemon is not running.")
	else
		http.write("<div class='table'>")

		http.write("<div class='row'>")
		current_header(tbl)
		http.write("</div>")

		http.write("<div class='row'>")
		current_body(tbl)
		http.write("</div>")

		http.write("</div>")
	end

	http.write("</div>")
end

function home.index()
	template.header()
	current()
	template.footer()
end

return home
