local http = { }

local jsonc = require "luci.jsonc"
local protocol = require "luci.http.protocol"

local io = {}
local context = {}

function http.io(recv, send)
	io.recv = recv
	io.send = send
end

local function subrange(t, from, to)
	local s = { }
	local last = to or #t

	for i = from, last do
		s[i - from + 1] = t[i]
	end

	return s
end

local function flush_header()
	for k, v in pairs(context.headers) do
		io.send(k)
		io.send(": ")
		io.send(v)
		io.send("\r\n")
	end
	io.send("\r\n")
	context.eoh = true
end

function http.close()
	if not context.eoh then
		flush_header()
	end

	context.closed = true
end

function http.header(key, value)
	if not context.headers then
		context.headers = {}
	end
	context.headers[key] = value
end

function http.status(code, message)
	code = code or 200
	message = message or protocol.statusmsg[code]

	http.header("Status", code .. " " .. message)
end

function http.content(mime)
	http.header("Content-Type", mime)
end

function http.write(x)
	if not x then
		http.close()
	else
		if not context.eoh then
			if not context.status then
				http.status()
			end
			if not context.headers or not context.headers["Content-Type"] then
				http.header("Content-Type", "text/html; charset=utf-8")
			end
			if not context.headers["Cache-Control"] then
				http.header("Cache-Control", "no-cache")
				http.header("Expires", "0")
			end

			flush_header()
		end
		io.send(x)
	end
end

function http.format(fmt, ...)
	http.write(string.format(fmt, ...))
end

function http.write_json(x)
	http.write(jsonc.stringify(x))
end

function http.dispatch(env)
	local params = {}

	-- Parameters
	if env.REQUEST_METHOD == "POST" then
		local buf = io.recv()

		protocol.urldecode_params(buf, params)
	else
		protocol.urldecode_params(env.QUERY_STRING, params)
	end

	-- Path
	local path = env.PATH_INFO or "/home"

	path = string.sub(path, 2)
	path = string.gsub(path, "/*$", "")

	local s = string.split(path, "/+", #path, true)

	local hasmod, mod = pcall(require, "wsview." .. s[1])
	local func = s[2] or "index"

	if hasmod and mod[func] then
		env.ARGS = subrange(s, 3)
		mod[func](env, params)
	else
		http.status(400)
	end

	http.close()
end

return http
