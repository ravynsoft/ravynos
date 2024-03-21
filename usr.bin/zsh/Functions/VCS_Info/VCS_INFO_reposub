## vim:ft=zsh
## Written by Frank Terbeck <ft@bewatermyfriend.org>
## Distributed under the same BSD-ish license as zsh itself.

setopt localoptions extendedglob NO_shwordsplit
local base=${1%%/##} tmp

tmp="$(pwd -P)"
[[ $tmp == ${base}/* ]] || {
    printf '.'
    return 1
}
printf '%s' ${tmp#$base/}
return 0
