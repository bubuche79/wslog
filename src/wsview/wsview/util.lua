local util = { }

function util.getconfig()
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

return util
