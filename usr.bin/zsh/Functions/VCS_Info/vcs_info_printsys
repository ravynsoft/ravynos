## vim:ft=zsh
## Written by Frank Terbeck <ft@bewatermyfriend.org>
## Distributed under the same BSD-ish license as zsh itself.

emulate -L zsh
setopt extendedglob

local sys
local -a disabled enabled
local -A vcs_comm

zstyle -a ":vcs_info:-init-:${1:-default}:-all-" "enable" enabled
(( ${#enabled} == 0 )) && enabled=( all )

if (( ${+VCS_INFO_backends} == 0 )); then
  autoload -Uz vcs_info_setsys
  vcs_info_setsys
fi

if [[ -n ${(M)enabled:#(#i)all} ]] ; then
    enabled=( ${VCS_INFO_backends} )
    zstyle -a ":vcs_info:-init-:${1:-default}:-all-" "disable" disabled
else
    for sys in ${VCS_INFO_backends} ; do
        [[ -z ${(M)enabled:#$sys} ]] && disabled+=( ${sys} )
    done
    enabled=( ${VCS_INFO_backends} )
fi

print -l '## list of supported version control backends:' \
         '## disabled systems are prefixed by a hash sign (#)'

for sys in ${VCS_INFO_backends} ; do
    [[ -n ${(M)disabled:#${sys}} ]] && printf '#'
    printf '%s\n' ${sys}
done

print -l '## flavours (cannot be used in the enable or disable styles; they' \
         '## are enabled and disabled with their master [git-svn -> git])'   \
         '## they *can* be used in contexts: '\'':vcs_info:git-svn:*'\''.'

for sys in ${VCS_INFO_backends} ; do
    VCS_INFO_detect_${sys} --flavours
done
return 0
