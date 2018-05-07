local http = { }

local json = require "json"
local util = require "wsview.util"

local io = {}
local context = {}

local statusmsg = {
	[200] = "OK",
	[206] = "Partial Content",
	[301] = "Moved Permanently",
	[302] = "Found",
	[304] = "Not Modified",
	[400] = "Bad Request",
	[403] = "Forbidden",
	[404] = "Not Found",
	[405] = "Method Not Allowed",
	[408] = "Request Time-out",
	[411] = "Length Required",
	[412] = "Precondition Failed",
	[416] = "Requested range not satisfiable",
	[500] = "Internal Server Error",
	[503] = "Server Unavailable"
}

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
	message = message or statusmsg[code]

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
	http.write(json.encode(x))
end

-- the "+" sign to " " - and return the decoded string.
local function urldecode( str, no_plus )

	local function __chrdec( hex )
		return string.char( tonumber( hex, 16 ) )
	end

	if type(str) == "string" then
		if not no_plus then
			str = str:gsub( "+", " " )
		end

		str = str:gsub( "%%([a-fA-F0-9][a-fA-F0-9])", __chrdec )
	end

	return str
end

-- from given url or string. Returns a table with urldecoded values.
-- Simple parameters are stored as string values associated with the parameter
-- name within the table. Parameters with multiple values are stored as array
-- containing the corresponding values.
local function urldecode_params( url, tbl )

	local params = tbl or { }

	if url:find("?") then
		url = url:gsub( "^.+%?([^?]+)", "%1" )
	end

	for pair in url:gmatch( "[^&;]+" ) do

		-- find key and value
		local key = urldecode( pair:match("^([^=]+)")     )
		local val = urldecode( pair:match("^[^=]+=(.+)$") )

		-- store
		if type(key) == "string" and key:len() > 0 then
			if type(val) ~= "string" then val = "" end

			if not params[key] then
				params[key] = val
			elseif type(params[key]) ~= "table" then
				params[key] = { params[key], val }
			else
				table.insert( params[key], val )
			end
		end
	end

	return params
end

function http.dispatch(env)
	local params = {}

	-- Parameters
	if env.REQUEST_METHOD == "POST" then
		local buf = io.recv()

		urldecode_params(buf, params)
	else
		urldecode_params(env.QUERY_STRING, params)
	end

	-- Path
	local path = env.PATH_INFO or "/home"

	path = string.sub(path, 2)
	path = string.gsub(path, "/*$", "")

	local s = util.split(path, "/+", #path, true)

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
