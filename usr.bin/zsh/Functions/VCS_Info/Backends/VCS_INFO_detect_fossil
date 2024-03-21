## vim:ft=zsh
## fossil support by: Mike Meyer <mwm@mired.org>
## Distributed under the same BSD-ish license as zsh itself.

setopt localoptions NO_shwordsplit

[[ $1 == '--flavours' ]] && return 1

VCS_INFO_check_com ${vcs_comm[cmd]} || return 1
vcs_comm[detect_need_file]="_FOSSIL_ .fslckout"
VCS_INFO_bydir_detect . || return 1

return 0
