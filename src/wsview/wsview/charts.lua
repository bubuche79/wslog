local charts = { }

local http = require "wsview.http"

function charts.add(ns, defs, p)
	local url

	if p.period then
		if p.period == 'month' then
			url = "month/" .. p.year .. "/" .. p.month
		else
			url = "year/" .. p.year
		end
	else
		url = "archive"
	end

	http.write([[<script src="https://cdnjs.cloudflare.com/ajax/libs/moment.js/2.20.1/moment-with-locales.min.js"></script>]])
	http.write([[<script src="https://cdnjs.cloudflare.com/ajax/libs/Chart.js/2.7.1/Chart.min.js"></script>]])
	http.write([[<div class="charts">]])

	for i = 1, #defs do
		local n = defs[i].name
		http.format([[<div class="chart"><canvas id="chart-%s"></canvas></div>]], n)
	end

	http.write([[</div><script>moment.locale('fr');]])

	for i = 1, #defs do
		local n = defs[i].name
		http.format([[var %s = document.getElementById("chart-%s").getContext("2d"); ]], n, n)
	end

	http.format([[fetch('/cgi-bin/wsview/rest/%s')]], url)
	http.write([[.then(function(response) { return response.json(); }).then(function(json) {]])

	for i = 1, #defs do
		local n = defs[i].name
		if p.period then
			http.format("new Chart(%s, %s_%s(json, '%s')); ", n, ns, n, p.period)
		else
			http.format("new Chart(%s, %s_%s(json)); ", n, ns, n)
		end
	end

	http.write([[});</script>]]);
end

return charts

