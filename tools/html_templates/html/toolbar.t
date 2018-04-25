%
<form method="post" accept-charset="UTF-8" enctype="multipart/form-data"
      action="<% change |s %>">
<table style="width: 100%"><tr><td>
% CALL toolbar_left
</td><td>
Change to:
<select name="folder" size="1">
% IFDEF $g_favourites[0]
%   FOREACH $m @g_favourites
%     IFEQ $m->name $g_preferred
<option value="<% $m->name |c %>" selected="selected"><% ${m->name|7} |h %></option>
%     ELSE
<option value="<% $m->name |c %>"><% ${m->name|7} |h %></option>
%     ENDIF
%   ENDFOREACH
% ELSE
%   FOREACH $m @g_list
<option value="<% $m->name |c %>"><% ${m->name|7} |h %></option>
%   ENDFOREACH
% ENDIF
</select>
<input type="submit" name="sub_folder_dialogue" value="Go" />
</td><td align="right">
% CALL toolbar_right
</td></tr></table>
</form>
