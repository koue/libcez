%
<table style="width: 100%">
% IFDEF $g_status
<tr class="t_status">
<td align="left"><b>Status:</b> <% ${g_status} |h %></td>
% ELSE
<tr class="t_nstatus">
<td>Default.</td>
% ENDIF
% IFDEF $g_service_name
<td align="right"><% "$g_service_name : $g_user" |h %></td>
% else
<td align="right">Templates: <% $g_user |h %></td>
% endif
</tr></table>
