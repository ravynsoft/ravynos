## vim:ft=zsh
## gnu arch support by: Frank Terbeck <ft@bewatermyfriend.org>
## Distributed under the same BSD-ish license as zsh itself.

setopt localoptions NO_shwordsplit

[[ $1 == '--flavours' ]] && return 1

VCS_INFO_check_com ${vcs_comm[cmd]} || return 1
vcs_comm[basedir]="$(${vcs_comm[cmd]} tree-root 2> /dev/null)" && return 0
return 1
