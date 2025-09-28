## vim:ft=zsh
## Written by Frank Terbeck <ft@bewatermyfriend.org>
## Distributed under the same BSD-ish license as zsh itself.

emulate -L zsh

local -i i
local -i maxexports

VCS_INFO_maxexports
for i in {0..$((maxexports - 1))} ; do
    printf -- '$vcs_info_msg_%d_: "' $i
    if zstyle -T ':vcs_info:formats:command:-all-' use-prompt-escapes ; then
        print -nP -- ${(P)${:-vcs_info_msg_${i}_}}
    else
        print -n -- ${(P)${:-vcs_info_msg_${i}_}}
    fi
    printf '"\n'
done
