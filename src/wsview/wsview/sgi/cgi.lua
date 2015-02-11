local cgi = { }

require "io"
require "os"
http = require "wsview.http"

function cgi.run()
	local env = {}

	env.CONTENT_LENGTH  = os.getenv("CONTENT_LENGTH")
	env.CONTENT_TYPE = os.getenv("CONTENT_TYPE")
	env.REQUEST_METHOD = os.getenv("REQUEST_METHOD")
	env.REQUEST_URI = os.getenv("REQUEST_URI")
	env.PATH_INFO  = os.getenv("PATH_INFO")
--	env.SCRIPT_NAME = os.getenv("SCRIPT_NAME"):gsub("/+$", "")
	env.SCRIPT_FILENAME = os.getenv("SCRIPT_NAME")
	env.SERVER_PROTOCOL = os.getenv("SERVER_PROTOCOL")
	env.QUERY_STRING = os.getenv("QUERY_STRING")

	http.io(io.read, io.write)
	http.dispatch(env)

	io.flush()
	io.close()
end

return cgi
