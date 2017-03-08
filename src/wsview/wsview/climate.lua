local http = require "wsview.http"

function climate_charts(year, month)
	http.write([[
<script type="text/javascript" src="/wsview-static/Chart.min.js"></script>
<div class="charts">
<div class="chart">
<canvas id="chart1"></canvas>
</div>
<div class="chart">
<canvas id="chart2"></canvas>
</div>
<div class="chart">
<canvas id="chart3"></canvas>
</div>
</div>
<script>
var ctx1 = document.getElementById("chart1").getContext("2d");
var ctx2 = document.getElementById("chart2").getContext("2d");
var ctx3 = document.getElementById("chart3").getContext("2d");

fetch('/cgi-bin/wsview/rest/climate/]] .. year .. '/' .. month .. [[/')
	.then(function(response) {
		return response.json();
	})
	.then(function(json) {
		new Chart(ctx1, chart_temp_rain(json));
		new Chart(ctx2, chart_wind(json));
		new Chart(ctx3, chart_barometer(json));
	});
</script>
]])
end

function climate(year, month)
	http.write([[
<ul class="tabs">
<li class="tabmenu-item"><a href="/cgi-bin/wsview/climate/table">Tableau</a></li>
<li class="tabmenu-item active"><a href="/cgi-bin/wsview/climate/">Graphiques</a></li>
</ul>]])

	climate_charts(year, month)
end


