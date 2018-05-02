%#
%# Arguments:
%#  $_cmd      Command
%#  $_icon     Icon name
%#  $_alt      Alt text for icon
%#
% IFDEF $g_use_icons
%   IFEQ $g_cmd $_cmd
<td align="center" valign="top">
 <img src="<% ${_icon}-inactive |i %>" alt="<% $_alt |h %>" /></td>
%   ELSE
<td align="center" valign="top"><a href="<% $_cmd |s %>">
 <img src="<% $_icon |i %>" alt="<% $_alt |h %>" /></a></td>
%   ENDIF
% ELSE
%   IFEQ $g_cmd $_cmd
<td align="center"><% $_alt |h %></td>
%   ELSE
<td align="center"><a href="<% $_cmd |s %>"><% $_alt |h %></a></td>
%   ENDIF
% ENDIF
