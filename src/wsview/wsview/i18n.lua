local i18n = { }

local conv = {
	temp = { unit = "°C", fmt = "%.1f" },
	humidity = { unit = "%", fmt = "%.0f" },
	rain_rate = { unit = "mm/h", fmt = "%.1f" },
	pressure = { unit = "hPa", fmt = "%.1f" },
	speed = { unit = "m/s", fmt = "%.1f" },
	dir = { fmt = "%s" }
}

local i18n = {
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

local lang = "fr"
local table = i18n[lang]

function i18n.getlang()
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

function i18n.translate(k)
	return table[k] or string.format("???%s???", k)
end

function i18n.unit(k)
	return conv[k]
end

function i18n.format_date(date)
	return os.date(table["date_fmt"], date)
end

return i18n
