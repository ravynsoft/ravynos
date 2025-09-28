## vim:ft=zsh
## subversion support by: Frank Terbeck <ft@bewatermyfriend.org>
## Distributed under the same BSD-ish license as zsh itself.

setopt localoptions NO_shwordsplit

[[ $1 == '--flavours' ]] && return 1

VCS_INFO_check_com ${vcs_comm[cmd]} || return 1
vcs_comm[detect_need_file]="entries format wc.db"
VCS_INFO_bydir_detect '.svn' || return 1
