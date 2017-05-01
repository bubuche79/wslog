require "luci.http.protocol"

conv = {
	temp = { unit = "°C", fmt = "%.1f" },
	humidity = { unit = "%", fmt = "%.0f" },
	rain_rate = { unit = "mm/h", fmt = "%.1f" },
	pressure = { unit = "hPa", fmt = "%.1f" },
	speed = { unit = "m/s", fmt = "%.1f" },
	dir = { fmt = "%s" }
}

i18n = {
	en = {
		temp = "Temperature",
		dew_point = "Dew point",
		humidity = "Humidity",
		rain = "Precipitation",
		wind_speed = "Wind speed",
		wind_gust = "Wind gust",
		wind_dir = "Wind direction",
		wind_from = "Wind from",
		barometer = "Pressure",
		current = "Observations",
		rain_rate = "Rain rate",
		feels = "Feels",
		date_fmt = "%B %d, %Y",
		summary = "Summary",
		tab_charts = "Graphs",
		tab_table = "Table",
		-- months
		january = "January",
		february = "February",
		march = "March",
		april = "April",
		may = "May",
		june = "June",
		july = "July",
		august = "August",
		september = "September",
		october = "October",
		november = "November",
		december = "December"
	},
	fr = {
		temp = "Température",
		dew_point = "Point de rosée",
		humidity = "Humidité",
		rain = "Précipitation",
		wind_speed = "Vitesse du vent",
		wind_gust = "Rafale de vent",
		wind_dir = "Direction du vent",
		wind_from = "Vent de",
		barometer = "Pression",
		current = "Observations",
		rain_rate = "Précipitation",
		feels = "Paraît",
		date_fmt = "%d %B %Y",
		summary = "Bilan",
		tab_charts = "Graphiques",
		tab_table = "Tableau",
		-- months
		january = "Janvier",
		february = "Février",
		march = "Mars",
		april = "Avril",
		may = "Mai",
		june = "Juin",
		july = "Juillet",
		august = "Août",
		september = "Septembre",
		october = "Octobre",
		november = "Novembre",
		december = "Décembre"
	}
}

function getlang()
	local lang = "en"
	local aclang = os.getenv("HTTP_ACCEPT_LANGUAGE") or ""

	for lpat in aclang:gmatch("[%w-]+") do
		lpat = lpat and lpat:gsub("-", "_")
		if i18n[lpat]~= nil then
			lang = lpat
			break
		end
	end

	return lang
end

function getparams()
	local qstring = os.getenv('QUERY_STRING') or ""

	return luci.http.protocol.urldecode_params(qstring)
end

function i18n_fmt(k)
	return i18n[lang][k] or string.format("???%s???", k)
end

function i18n_date(date)
	return os.date(i18n[lang]['date_fmt'], date)
end

function printf(s,...)
	return print(s:format(...))
end

function getconfig()
	local property = {}
	local file = io.open("/etc/wslogd.conf", "r")

	for line in file:lines() do
		key, value = string.match(line, "^(.-) *= *(.-)$")

		if (key ~= nil) then
			property[key] = value
		end
	end

	file:close()

	return property
end

function string.starts(str, substr)
	return string.sub(str, 1, string.len(substr)) == substr
end

function string.split(str, char)
	local t = {}
	string.gsub(str, '(.-)' .. char, function(a) table.insert(t, a) end)
	return t
end
