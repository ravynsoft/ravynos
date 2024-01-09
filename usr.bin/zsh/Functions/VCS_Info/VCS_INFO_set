## vim:ft=zsh
## Written by Frank Terbeck <ft@bewatermyfriend.org>
## Distributed under the same BSD-ish license as zsh itself.

# This function sets ${vcs_info_msg_<N>_} from ${msgs}.

setopt localoptions noksharrays NO_shwordsplit unset
local -i i j

if [[ $1 == '--nvcs' ]] ; then
    [[ $2 == '-preinit-' ]] && (( maxexports == 0 )) && (( maxexports = 1 ))
    for i in {0..$((maxexports - 1))} ; do
        typeset -g vcs_info_msg_${i}_=
    done
    VCS_INFO_nvcsformats $2
    [[ $2 != '-preinit-' ]] && VCS_INFO_hook "no-vcs"
fi

(( ${#msgs} - 1 < 0 )) && return 0
for i in {0..$(( ${#msgs} - 1 ))} ; do
    (( j = i + 1 ))
    typeset -g vcs_info_msg_${i}_=${msgs[$j]}
done

if (( i < maxexports )) ; then
    for j in {$(( i + 1 ))..${maxexports}} ; do
        [[ -n ${(P)${:-vcs_info_msg_${j}_}} ]] && typeset -g vcs_info_msg_${j}_=
    done
fi
return 0
