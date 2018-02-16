local cgi = { }

require "io"
require "luci.sys"
http = require "wsview.http"

function cgi.run()
	local env = luci.sys.getenv()

	http.io(nil, io.write)
	http.dispatch(env)

	io.flush()
	io.close()
end

return cgi
