% if not defined('in_page'):
<!DOCTYPE html>
<html>
  <head>
    <link rel="stylesheet" href="static/css/bootstrap.min.css" integrity="sha384-1q8mTJOASx8j1Au+a5WDVnPi2lkFfwwEAa8hDDdjZlpLegxhjVME1fgjWPGmkzs7" crossorigin="anonymous"/>
    <link rel="stylesheet" href="/static/css/header.css"/>
    <script type="text/javascript" src="/static/js/jquery-1.12.0.min.js"></script>
    <script type="text/javascript" src="/static/js/bootstrap.min.js"></script>
    <title>dynStruct</title>
  </head>
  <body>
    <div id="navbar" class="navbar navbar-default navbar-static-top">
      <div class="collapse navbar-collapse">
	<ul class="nav navbar-nav">
	  <li> <a href="/struct_search">Structures</a></li>
	  <li> <a href="/block_search">Blocks</a></li>
	  <li> <a href="/access_search">Access</a></li>
	  <li> <a href="/struct_create">Create Structure</a></li>
	  <li> <a href="/header.h">Download Header</a></li>
	  <li> <a href="/quit">Stop dynStruct.py</a></li>
	</ul>
      </div>
    </div>
% end
