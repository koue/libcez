%#
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
        "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" lang="en" xml:lang="en">
<head>
% IFDEF $g_service_name
<title><% $g_service_name |h %> templates</title>
% ELSE
<title>Testing templates</title>
% ENDIF
<meta name="robots" content="none" />
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
<link rel="stylesheet" href="/static/common.css" type="text/css" />
<link rel="stylesheet"
      href="/static/<% $g_theme->name |n %>.css"
      type="text/css" />
<link rel="icon" type="image/png"  href="/icons/favicon.png" />
</head>
<body>
