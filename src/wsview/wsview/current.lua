require "wsview"
require "wsview.util"

function div_row1(id, v, unit, label, split, el)
	local fmt = conv[unit]

	el = el or "span"
	split = split or 1

	print("<div id='rt-" .. id .. "' class='row'>")

	print("<div class='col'>")
	if (label ~= nil) then
		print("<span class='ws-label'>" .. i18n_fmt(label) .. "</span>")
		if (split == 1) then
			print("</div><div class='col'>")
		end
	end
	printf("<%s class='ws-data'>", el)
	if (v ~= nil) then
		printf("<%s class='ws-value'>" .. fmt.fmt .. "</%s>", el, v, el)
		if (fmt.unit ~= nil) then
			printf("<%s class='ws-unit'>%s</%s>", el, fmt.unit, el)
		end
	else
		printf("<%s class='ws-value'>--</%s>", el, el)
	end
	printf("</%s></div>", el)

	print("</div>")
end

function div_row(metric, unit, tbl, label, split, el)
	local id = string.gsub(metric, "_", "-")
	local v = tbl[metric]

	div_row1(id, v, unit, label, split, el)
end

function curr_hrow(metric, unit, tbl, label)
	div_row(metric, unit, tbl, label, 0)
end

function curr_row(metric, unit, tbl)
	div_row(metric, unit, tbl, metric)
end

function current_header(tbl)
	local dir

	-- temp
	print("<div class='col'>")
	print("<div class='table'>")
	curr_hrow("temp", "temp", tbl)
	curr_hrow("windchill", "temp", tbl, "feels")
	print("</div>")
	print("</div>")

	-- wind
	print("<div class='col'>")
	print("<div class='table'>")

	print("<div id='rt-wind' class='row'>")
	print("<div class='col'>")
	if (tbl.wind_dir ~= nil) then
		dir = ws_wind_dir(tbl.wind_dir)

		printf("<div style='transform: rotate(%ddeg); '>", tbl.wind_dir)
		print("<div class='circle'><div class='arrow'></div></div>")
		print("</div>")
	else
		dir = "--"
	end
	div_row("wind_speed", "speed", tbl, nil, 0, "div")
	print("</div>")
	print("</div>")

	div_row1("wind-dir", dir, "dir", "wind_from", 0)

	print("</div>")
	print("</div>")
end

function current_body(tbl)
	print("<div class='col'>")
	print("<div id='rt-tbody' class='table'>")
	curr_row("dew_point", "temp", tbl)
	curr_row("humidity", "humidity", tbl)
	curr_row("rain_rate", "rain_rate", tbl)
	curr_row("barometer", "pressure", tbl)
	print("</div>")
	print("</div>")
end

function current()
	local tbl = ws_current()

	print("<h2>" .. i18n_fmt('current') .. "</h2>")
	print("<div id='current'>")

	if (tbl == nil) then
		print("The wslogd daemon is not running")
	else
		print("<div class='table'>")

		print("<div class='row'>")
		current_header(tbl)
		print("</div>")

		print("<div class='row'>")
		current_body(tbl)
		print("</div>")

		print("</div>")
	end

	print("</div>")
end

