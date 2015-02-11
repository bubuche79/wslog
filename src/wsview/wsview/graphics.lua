local graphics = { }

local http = require "wsview.http"
local i18n = require "wsview.i18n"

local function url_path(p)
	local path

	if p.period then
		if p.period == 'month' then
			path = "aggregate/day/" .. p.year .. "/" .. p.month
		else
			path = "aggregate/month/" .. p.year
		end
	else
		if p.year then
			path = "archive/" .. p.year .. "/" .. p.month .. "/" .. p.day
		else
			path = "archive"
		end
	end

	return path
end

function graphics.tab(tab, url, active)
	local class = ""
	local i18n_tab = i18n.translate("tab_" .. tab)

	if tab == active then
		class = " active"
	end

	http.format("<li class='tabmenu-item%s'>", class)
	http.format("<a href='%s/%s'>%s</a>", url, tab, i18n_tab)
	http.write("</li>")
end

function graphics.charts(ns, defs, p)
	local path = url_path(p)

	http.write([[<div class="graphics-charts">]])
	http.write([[<script src="https://cdnjs.cloudflare.com/ajax/libs/moment.js/2.22.2/moment-with-locales.min.js"></script>]])
	http.write([[<script src="https://cdnjs.cloudflare.com/ajax/libs/Chart.js/2.7.2/Chart.min.js"></script>]])

	for i = 1, #defs do
		local n = defs[i].name
		http.format([[<div class="chart"><canvas id="chart-%s"></canvas></div>]], n)
	end

	http.write([[<script>moment.locale('fr');]])

	for i = 1, #defs do
		local n = defs[i].name
		http.format([[var %s = document.getElementById("chart-%s").getContext("2d"); ]], n, n)
	end

	http.format([[fetch('/cgi-bin/wsview/rest/%s')]], path)
	http.write([[.then(function(response) { return response.json(); }).then(function(json) {]])

	for i = 1, #defs do
		local n = defs[i].name
		if p.period then
			http.format("new Chart(%s, %s_%s(json, '%s')); ", n, ns, n, p.period)
		else
			http.format("new Chart(%s, %s_%s(json)); ", n, ns, n)
		end
	end

	http.write([[});</script></div>]]);
end

function graphics.table(defs, p)
	local path = url_path(p)

	http.write([[<div id="graphics-tables" class="tables">]])
	http.write([[<script src="https://cdnjs.cloudflare.com/ajax/libs/moment.js/2.22.2/moment-with-locales.min.js"></script>]])
	http.write([[<script>moment.locale('fr');]])
	http.format([[var root = document.getElementById("graphics-tables");]])
	http.format([[fetch('/cgi-bin/wsview/rest/%s')]], path)
	http.write([[.then(function(response) { return response.json(); }).then(function(json) {]])
	http.format([[table_%s(root, json);});</script>]], defs.name)
end

return graphics

