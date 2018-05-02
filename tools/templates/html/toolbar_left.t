%#
%
<table>
<tr>
% CALL tb_icon, $_cmd => "display", $_icon => "display", $_alt => "Message"
<td></td>
% CALL tb_icon, $_cmd => "list", $_icon => "list", $_alt => "List"
<td></td>
% CALL tb_icon, $_cmd => "manage", $_icon => "manage", $_alt => "Manage"
</tr>
% IFDEF $g_use_icons
<tr>
<td align="center" valign="top">Message</td>
<td>|</td>
<td align="center" valign="top">List</td>
<td>|</td>
<td align="center" valign="top">Manage</td>
</tr>
% ENDIF
</table>
