## vim:ft=zsh
## Written by Frank Terbeck <ft@bewatermyfriend.org>
## Distributed under the same BSD-ish license as zsh itself.

setopt localoptions NO_shwordsplit

case $1 in
    (/*)
        [[ -x $1 ]] && return 0
        ;;
    (*)
        (( ${+commands[$1]} )) && return 0
esac

return 1
