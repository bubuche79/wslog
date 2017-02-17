require "luci.http.protocol"

i18n = {
	en = {
		temp = "Temperature",
		dew_point = "Dew point",
		humidity = "Humidity",
		rain = "Precipitation",
		date_fmt = "%B %d, %Y"
	},
	fr = {
		temp = "Température",
		dew_point = "Point de rosée",
		humidity = "Humidité",
		rain = "Précipitation",
		wind_speed = "Vitesse du vent",
		wind_gust = "Rafale de vent",
		wind_dir = "Direction du vent",
		barometer = "Pression",
		current = "Observations",
		date_fmt = "%d %B %Y"
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
