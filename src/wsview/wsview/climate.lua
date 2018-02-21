local climate = { }

local http = require "wsview.http"
local template = require "wsview.template"
local i18n = require "wsview.i18n"

local function add_tab(view, tab, active)
	local class = ""
	local i18n_tab = i18n.translate("tab_" .. tab)

	if (active == tab) then
		class = " active"
	end

	http.format("<li class='tabmenu-item%s'>", class)
	http.format("<a href='/cgi-bin/wsview/climate/%s/%s'>%s</a>", view, tab, i18n_tab)
	http.write("</li>")
end

local function add_charts(n)
	local i

	http.write('<script src="https://cdnjs.cloudflare.com/ajax/libs/moment.js/2.20.1/moment.min.js"></script>')
	http.write('<script src="https://cdnjs.cloudflare.com/ajax/libs/Chart.js/2.7.1/Chart.min.js"></script>')
	http.write('<div class="charts">')
	for i = 1,n do
		http.write('<div class="chart"><canvas id="chart%d/></div>', i)
	end
	http.write('</div>')
	http.write('<script>')
end

local function climate_charts(year, month)
	http.write([[
<script src="https://cdnjs.cloudflare.com/ajax/libs/moment.js/2.20.1/moment.min.js"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/Chart.js/2.7.1/Chart.min.js"></script>
<div class="charts">
<div class="chart">
<canvas id="chart1"></canvas>
</div>
<div class="chart">
<canvas id="chart2"></canvas>
</div>
</div>
<script>
var ctx1 = document.getElementById("chart1").getContext("2d");
var ctx2 = document.getElementById("chart2").getContext("2d");
fetch('/cgi-bin/wsview/rest/year/')
	.then(function(response) {
		return response.json();
	})
	.then(function(json) {
//		new Chart(ctx2, chart_wind(json));
//		new Chart(ctx3, chart_barometer(json));
		new Chart(ctx1, chart_year_temp(json));
		new Chart(ctx2, chart_year_rain(json));
	});
</script>]])
end

local function climate_table(year, month)
end

local function add_input_year(selected, last)
	http.write([[<select class="cbi-input-select" name="year">]])

	for i=2017,last,1 do
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

local function climate1(env, p)
	local t = os.date("*t")

	local view = p.view or "charts"
	local year = p.year or t.year
	local month = p.month or t.month

	http.write([[<ul class="tabs">]])
	add_tab("charts", view)
	add_tab("table", view)

	http.write([[</ul><div class="tab-main">
<div class="climate-period">
<form name="test" action="/cgi-bin/wsview/climate/">]])
	add_input_year(year, t.year)
	if p.period == 'month' then
		add_input_month(month)
	end

	http.write([[<input class="cbi-button" type="submit" value="Submit"></form></div>]])

	if (view == "table") then
		climate_table(year, month)
	else
		climate_charts(year, month)
	end

	http.write([[</div>]])
end

function climate.month(env)
	template.header()
	climate1(env, { period = "month" })
	template.footer()
end

function climate.year(env)
	template.header()
	climate1(env, { period = "year" })
	template.footer()
end

return climate

