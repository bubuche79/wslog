local util = { }

local conf = nil

function util.getconf(k)
	if not conf then
		conf = util.getconfig()
	end

	return conf[k]
end

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

function util.split(str, pat, max, regex)
	pat = pat or "\n"
	max = max or #str

	local t = {}
	local c = 1

	if #str == 0 then
		return {""}
	end

	if #pat == 0 then
		return nil
	end

	if max == 0 then
		return str
	end

	repeat
		local s, e = str:find(pat, c, not regex)
		max = max - 1
		if s and max < 0 then
			t[#t+1] = str:sub(c)
		else
			t[#t+1] = str:sub(c, s and s - 1)
		end
		c = e and e + 1 or #str + 1
	until not s or max < 0

	return t
end

return util
