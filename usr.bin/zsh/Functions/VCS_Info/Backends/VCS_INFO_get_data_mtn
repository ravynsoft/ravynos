## vim:ft=zsh
## monotone support by: Frank Terbeck <ft@bewatermyfriend.org>
## Distributed under the same BSD-ish license as zsh itself.

setopt localoptions extendedglob NO_shwordsplit
local mtnbranch mtnbase

mtnbase=${vcs_comm[basedir]}
rrn=${mtnbase:t}
mtnbranch=${${(M)${(f)"$( ${vcs_comm[cmd]} status )"}:#(#s)Current branch:*}/*: /}
VCS_INFO_formats '' "${mtnbranch}" "${mtnbase}" '' '' '' ''
return 0
