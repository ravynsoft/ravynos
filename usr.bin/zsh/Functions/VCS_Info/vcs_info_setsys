## vim:ft=zsh
## Written by Frank Terbeck <ft@bewatermyfriend.org>
## Distributed under the same BSD-ish license as zsh itself.

emulate -L zsh
setopt extendedglob typeset_silent

local sys
typeset -ga VCS_INFO_backends
local -a match mbegin mend

VCS_INFO_backends=()

for file in ${^fpath}/VCS_INFO_get_data_*~*(\~|.zwc)(N) ; do
    file=${file:t}
    : ${file:#(#b)VCS_INFO_get_data_(*)}
    sys=${match[1]}

    [[ -n ${(M)VCS_INFO_backends:#${sys}} ]] && continue
    VCS_INFO_backends+=(${sys})
    autoload -Uz VCS_INFO_detect_${sys}
    autoload -Uz VCS_INFO_get_data_${sys}
done

return 0
