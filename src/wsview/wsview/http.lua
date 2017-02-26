local coroutine = require "coroutine"
local jsonc = require "luci.jsonc"

module "wsview.http"

local context = {}

function close()
	if not context.eoh then
		context.eoh = true
		coroutine.yield(3)
	end

	if not context.closed then
		context.closed = true
		coroutine.yield(5)
	end
end

function header(key, value)
	if not context.headers then
		context.headers = {}
	end
	context.headers[key:lower()] = value
	coroutine.yield(2, key, value)
end

function prepare_content(mime)
	if not context.headers or not context.headers["content-type"] then
		header("Content-Type", mime)
	end
end

function status(code, message)
	code = code or 200
	message = message or "OK"
	context.status = code
	coroutine.yield(1, code, message)
end

function write(content, src_err)
	if not content then
		if src_err then
			error(src_err)
		else
			close()
		end
		return true
	elseif #content == 0 then
		return true
	else
		if not context.eoh then
			if not context.status then
				status()
			end
			if not context.headers or not context.headers["content-type"] then
				header("Content-Type", "text/html; charset=utf-8")
			end
			if not context.headers["cache-control"] then
				header("Cache-Control", "no-cache")
				header("Expires", "0")
			end


			context.eoh = true
			coroutine.yield(3)
		end
		coroutine.yield(4, content)
		return true
	end
end

function write_json(x)
	write(jsonc.stringify(x))
end
