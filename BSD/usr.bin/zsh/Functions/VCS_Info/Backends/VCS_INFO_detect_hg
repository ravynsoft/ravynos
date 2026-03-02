## vim:ft=zsh
## mercurial support by: Frank Terbeck <ft@bewatermyfriend.org>
## Distributed under the same BSD-ish license as zsh itself.

setopt localoptions NO_shwordsplit

[[ $1 == '--flavours' ]] && { print -l hg-git hg-hgsubversion hg-hgsvn; return 0 }

VCS_INFO_check_com ${vcs_comm[cmd]} || return 1
vcs_comm[detect_need_file]="store data sharedpath"
VCS_INFO_bydir_detect '.hg' || return 1

if [[ -d ${vcs_comm[basedir]}/.hg/svn ]] ; then
    vcs_comm[overwrite_name]='hg-hgsubversion'
elif [[ -d ${vcs_comm[basedir]}/.hgsvn ]] ; then
    vcs_comm[overwrite_name]='hg-hgsvn'
elif [[ -e ${vcs_comm[basedir]}/.hg/git-mapfile ]] ; then
    vcs_comm[overwrite_name]='hg-git'
fi
return 0
