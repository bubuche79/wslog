require "wsview"
require "wsview.util"

local conv = {
	temp = { unit = "°C", fmt = "%.1f" },
	humidity = { unit = "%", fmt = "%d" },
	rain_rate = { unit = "mm/h", fmt = "%.1f" },
	pressure = { unit = "hPa", fmt = "%.1f" }
}

function div_row(metric, unit, tbl, label, split)
	local v = tbl[metric]
	local fmt = conv[unit]

	print("<div id='cur-" .. string.gsub(metric, "_", "-") .. "' class='row'>")

	print("<div class='col'>");
	if (label ~= nil) then
		print("<span class='ws-label'>" .. i18n_fmt(label) .. "</span>")
		if (split == 1) then
			print("</div>")
			print("<div class='col'>")
		end
	end
	print("<span class='ws-data'>")
	if (v ~= nil) then
		print(string.format("<span class='ws-value'>" .. fmt.fmt .. "</span>", v))
		print("<span class='ws-unit'>" .. fmt.unit .. "</span>")
	else
		print("<span class='ws-value'>--</span>")
	end
	print("</span>")
	print("</div>")

	print("</div>")
end

function curr_hrow(metric, unit, tbl, label)
	div_row(metric, unit, tbl, label, 0)
end

function curr_row(metric, unit, tbl)
	div_row(metric, unit, tbl, metric, 1)
end

function current()
	local tbl = ws_current();

	print("<h2 class='current'>" .. i18n_fmt('current') .. "</h2>")
	print("<div id='current'>")

	if (tbl == nil) then
		print("The wslogd daemon is not running")
	else
		print("<div class='table'>");
		curr_hrow("temp", "temp", tbl);
		curr_hrow("windchill", "temp", tbl, "feels");
		print("</div>");

		print("<div class='table'>");
		curr_row("dew_point", "temp", tbl);
		curr_row("humidity", "humidity", tbl);
		curr_row("rain_rate", "rain_rate", tbl);
		curr_row("barometer", "pressure", tbl);
		print("</div>");
	end

	print("</div>")
end

