## vim:ft=zsh
## Written by Frank Terbeck <ft@bewatermyfriend.org>
## Distributed under the same BSD-ish license as zsh itself.

emulate -L zsh
setopt extendedglob

if (( ${#argv} < 2 )); then
    print 'usage: vcs_info_hookadd <HOOK> <FUNCTION(s)...>'
    return 1
fi

local hook func context
local -a old

hook=$1
shift
context=":vcs_info-static_hooks:${hook}"

zstyle -a "${context}" hooks old
zstyle "${context}" hooks "${old[@]}" "$@"
return $?
