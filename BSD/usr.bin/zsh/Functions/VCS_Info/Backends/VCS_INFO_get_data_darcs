## vim:ft=zsh
## darcs support by: Frank Terbeck <ft@bewatermyfriend.org>
## Distributed under the same BSD-ish license as zsh itself.

setopt localoptions NO_shwordsplit
local darcsbase

darcsbase=${vcs_comm[basedir]}
rrn=${darcsbase:t}
VCS_INFO_formats '' "${darcsbase:t}" "${darcsbase}" '' '' '' ''
return 0
