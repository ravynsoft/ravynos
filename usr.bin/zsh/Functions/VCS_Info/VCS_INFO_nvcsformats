## vim:ft=zsh
## Written by Frank Terbeck <ft@bewatermyfriend.org>
## Distributed under the same BSD-ish license as zsh itself.

setopt localoptions noksharrays NO_shwordsplit
local c v rr

if [[ $1 == '-preinit-' ]] ; then
    c='default'
    v='-preinit-'
    rr='-all-'
fi
zstyle -a ":vcs_info:${v:-$vcs}:${c:-$usercontext}:${rrn:-$rr}" nvcsformats msgs
(( ${#msgs} > maxexports )) && msgs[${maxexports},-1]=()
return 0
