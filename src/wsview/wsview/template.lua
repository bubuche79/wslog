local template = { }

local http = require "wsview.http"

function template.header()
	http.content("text/html; charset=utf-8")

	http.write([[<!DOCTYPE html>
<html>
<head>
 <meta charset="utf-8">
 <link type="text/css" rel="stylesheet" href="/luci-static/bootstrap/cascade.css">
 <link type="text/css" rel="stylesheet" href="/wsview-static/wsview.css">
 <script type="text/javascript" src="/wsview-static/wsview.js"></script>
</head>
<body>
<header>
<div class="fill">
<div class="container">
<a class="brand" href="#">WSVIEW</a>
<ul class="nav">
 <li><a href="/cgi-bin/wsview/">Temps réel</a></li>
 <li><a href="/cgi-bin/wsview/archive/">Observations</a></li>
 <li class="dropdown">
  <a class="menu" href="#">Climat</a>
  <ul class="dropdown-menu">
   <li><a href="/cgi-bin/wsview/climate/year">Annuel</a></li>
   <li><a href="/cgi-bin/wsview/climate/month">Mensuel</a></li>
  </ul>
 </li>
</ul>
</div>
</div>
</header>
<div class="container">
]])
end

function template.footer()
	http.write([[</div></body></html>]])
end

return template
