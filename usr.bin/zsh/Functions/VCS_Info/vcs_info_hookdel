## vim:ft=zsh
## Written by Frank Terbeck <ft@bewatermyfriend.org>
## Distributed under the same BSD-ish license as zsh itself.

emulate -L zsh
setopt extendedglob

local -i all

if [[ "x$1" == 'x-a' ]]; then
    all=1
    shift
else
    all=0
fi

if (( ${#argv} < 2 )); then
    print 'usage: vcs_info_hookdel [-a] <HOOK> <FUNCTION(s)...>'
    return 1
fi

local hook func context
local -a old

hook=$1
shift
context=":vcs_info-static_hooks:${hook}"

zstyle -a "${context}" hooks old || return 0
for func in "$@"; do
    if [[ -n ${(M)old:#$func} ]]; then
        old[(Re)$func]=()
    else
        printf 'Not statically registered to `%s'\'': "%s"\n' \
            "${hook}" "${func}"
        continue
    fi
    if (( all )); then
        while [[ -n ${(M)old:#$func} ]]; do
            old[(Re)$func]=()
        done
    fi
done
zstyle "${context}" hooks "${old[@]}"
return $?
