local climate = { }

local http = require "wsview.http"
local template = require "wsview.template"
local i18n = require "wsview.i18n"
local graphics = require "wsview.graphics"

local function climate_charts(p)
	local defs = { }

	if p.period == 'month' then
		defs[1] = { name = "temp_rain" }
		defs[2] = { name = "wind" }
		defs[3] = { name = "barometer" }
	else
		defs[1] = { name = "temp" }
		defs[2] = { name = "rain" }
	end

	graphics.charts("aggr", defs, p)
end

local function climate_table(p)
	graphics.table({ name = "aggr" }, p)
end

local function add_input_year(selected, last)
	http.write([[<select class="cbi-input-select" name="year">]])

	for i=2018,last,1 do
		if (i == selected) then
			http.write(string.format([[<option value="%d" selected>%d</option>]], i, i))
		else
			http.write(string.format([[<option value="%d">%d</option>]], i, i))
		end
	end

	http.write([[</select>]])
end

local function add_input_month(selected)
	local array = {
		i18n.translate('january'),
		i18n.translate('february'),
		i18n.translate('march'),
		i18n.translate('april'),
		i18n.translate('may'),
		i18n.translate('june'),
		i18n.translate('july'),
		i18n.translate('august'),
		i18n.translate('september'),
		i18n.translate('october'),
		i18n.translate('november'),
		i18n.translate('december')
	}

	http.write([[<select class="cbi-input-select" name="month">]])

	for i=1,12,1 do
		local month = array[i]

		if (i == selected) then
			http.format([[<option value="%d" selected>%s</option>]], i, month)
		else
			http.format([[<option value="%d">%s</option>]], i, month)
		end
	end

	http.write([[</select>]])
end

local function main(env, p)
	local t = os.date("*t")
	local prefix = string.format("/cgi-bin/wsview/climate/%s", p.period)

	p.year = tonumber(p.year) or t.year
	p.month = tonumber(p.month) or t.month

	http.write([[<ul class="tabs">]])
	graphics.tab("charts", prefix, p.view)
	graphics.tab("table", prefix, p.view)

	http.write([[</ul><div class="tab-main"><div class="climate-period">]])
	http.format([[<form method="POST" action="%s">]], prefix)
	add_input_year(year, t.year)
	if p.period == 'month' then
		add_input_month(p.month)
	end

	http.format([[<input type="hidden" name="view" value="%s">]], p.view)
	http.write([[<input class="cbi-button" type="submit" value="Submit"></form></div>]])

	if (p.view == "table") then
		climate_table(p)
	else
		climate_charts(p)
	end

	http.write([[</div>]])
end

function climate.month(env, params)
	template.header()
	if not params.view then
		params.view = env.ARGS[1] or "charts"
	end
	params.period = "month"
	main(env, params)
	template.footer()
end

function climate.year(env, params)
	template.header()
	if not params.view then
		params.view = env.ARGS[1] or "charts"
	end
	params.period = "year"
	main(env, params)
	template.footer()
end

return climate

