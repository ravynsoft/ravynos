## vim:ft=zsh
## gnu arch support by: Frank Terbeck <ft@bewatermyfriend.org>
## Distributed under the same BSD-ish license as zsh itself.

setopt localoptions extendedglob NO_shwordsplit
local tlabase tlabranch

tlabase=${vcs_comm[basedir]:P}
rrn=${tlabase:t}
# tree-id gives us something like 'foo@example.com/demo--1.0--patch-4', so:
tlabranch=${${"$( ${vcs_comm[cmd]} tree-id )"}/*\//}
VCS_INFO_formats '' "${tlabranch}" "${tlabase}" '' '' '' ''
return 0
