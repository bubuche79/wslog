require "wsview.sgi.cgi"

function handle_request(env)
	local renv = {
		CONTENT_LENGTH  = env.CONTENT_LENGTH,
		CONTENT_TYPE    = env.CONTENT_TYPE,
		REQUEST_METHOD  = env.REQUEST_METHOD,
		REQUEST_URI     = env.REQUEST_URI,
		PATH_INFO	= env.PATH_INFO,
		SCRIPT_NAME     = env.SCRIPT_NAME:gsub("/+$", ""),
		SCRIPT_FILENAME = env.SCRIPT_NAME,
		SERVER_PROTOCOL = env.SERVER_PROTOCOL,
		QUERY_STRING    = env.QUERY_STRING
	}

	local k, v
	for k, v in pairs(env.headers) do
		k = k:upper():gsub("%-", "_")
		renv["HTTP_" .. k] = v
	end

	local len = tonumber(env.CONTENT_LENGTH) or 0
	local function recv()
		if len > 0 then
			local rlen, rbuf = uhttpd.recv(4096)
			if rlen >= 0 then
				len = len - rlen
				return rbuf
			end
		end
		return nil
	end

	local send = uhttpd.send

	-- Process request
	wsview.sgi.cgi.execute(recv, send, renv)
end
