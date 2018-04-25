%#
% CALL header
% CALL toolbar
% CALL status
% IFDEF $g_service_name
<h2 style="text-align: center">Welcome to <% $g_service_name |h %> templates
% ELSE
<h2 style="text-align: center">Welcome to Testing templates
% ENDIF
</h2>
% IFDEF $g_help
%   CALL welcome_help
% ENDIF
% CALL footer
