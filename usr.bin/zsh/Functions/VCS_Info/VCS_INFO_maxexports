## vim:ft=zsh
## Written by Frank Terbeck <ft@bewatermyfriend.org>
## Distributed under the same BSD-ish license as zsh itself.

setopt localoptions NO_shwordsplit unset

zstyle -s ":vcs_info:${vcs}:${usercontext}:${rrn}" "max-exports" maxexports || maxexports=2
if [[ ${maxexports} != <-> ]] || (( maxexports < 1 )); then
    printf 'vcs_info(): expecting numeric arg >= 1 for max-exports (got %s).\n' ${maxexports}
    printf 'Defaulting to 2.\n'
    maxexports=2
fi
return 0
