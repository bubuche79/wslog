require "wsview.util"
local http = require "wsview.http"

function add_tab(tab, active)
	local class = ""

	if (active == tab) then
		class = " active"
	end

	http.write(string.format("<li class='tabmenu-item%s'>", class))
	http.write(string.format("<a href='/cgi-bin/wsview/climate/%s'>%s</a>", tab, i18n_fmt("tab_" .. tab)))
	http.write([[</li>]])
end

function climate_charts(year, month)
	http.write([[
<script type="text/javascript" src="/wsview-static/Chart.min.js"></script>
<div class="charts">
<div class="chart"><canvas id="chart1"></canvas></div>
<div class="chart"><canvas id="chart2"></canvas></div>
</div>
<script>
var ctx1 = document.getElementById("chart1").getContext("2d");
var ctx2 = document.getElementById("chart2").getContext("2d");

fetch('/cgi-bin/wsview/rest/climate/]] .. year .. [[/')
	.then(function(response) {
		return response.json();
	})
	.then(function(json) {
		new Chart(ctx1, chart_year_temp(json));
		new Chart(ctx2, chart_year_rain(json));
	});
</script>]])
end

function climate_table(year, month)
end

function add_input_year(selected, last)
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

function climate(params)
	local t = os.date("*t")

	local view = params.view or "charts"
	local year = tonumber(params.year) or t.year

	http.write([[<ul class="tabs">]])
	add_tab("charts", view)
	add_tab("table", view)

	http.write([[</ul><div class="tab-main">
<div class="climate-period">
<form name="test" action="/cgi-bin/wsview/climate/">]])
	add_input_year(year, t.year)

	http.write([[<input class="cbi-button" type="submit" value="Submit"></form></div>]])

	if (view == "table") then
		climate_table(year, month)
	else
		climate_charts(year, month)
	end

	http.write([[</div>]])
end


