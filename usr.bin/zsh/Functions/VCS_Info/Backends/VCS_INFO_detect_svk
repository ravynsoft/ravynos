## vim:ft=zsh
## svk support by: Frank Terbeck <ft@bewatermyfriend.org>
## Distributed under the same BSD-ish license as zsh itself.

setopt localoptions NO_shwordsplit

[[ $1 == '--flavours' ]] && return 1

# This detection function is a bit different from the others.
# We need to read svk's config file to detect a svk repository
# in the first place. Therefore, we'll just proceed and read
# the other information, too. This is more then any of the
# other detections do but this takes only one file open for
# svk at most. VCS_INFO_get_data_svk() gets simpler, too. :-)

setopt localoptions noksharrays extendedglob
local -i fhash
fhash=0

VCS_INFO_check_com ${vcs_comm[cmd]} || return 1
[[ -f ~/.svk/config ]] || return 1

while IFS= read -r line ; do
    if [[ -n ${vcs_comm[basedir]} ]] ; then
        line=${line## ##}
        [[ ${line} == depotpath:* ]] && vcs_comm[branch]=${line##*/}
        [[ ${line} == revision:* ]] && vcs_comm[revision]=${line##*[[:space:]]##}
        [[ -n ${vcs_comm[branch]} ]] && [[ -n ${vcs_comm[revision]} ]] && break
        continue
    fi
    (( fhash > 0 )) && [[ ${line} == '  '[^[:space:]]*:* ]] && break
    [[ ${line} == '  hash:'* ]] && fhash=1 && continue
    (( fhash == 0 )) && continue
    [[ ${PWD}/ == ${${line## ##}%:*}/* ]] && vcs_comm[basedir]=${${line## ##}%:*}
done < ~/.svk/config

[[ -n ${vcs_comm[basedir]} ]]  && \
[[ -n ${vcs_comm[branch]} ]]   && \
[[ -n ${vcs_comm[revision]} ]] && return 0
return 1
