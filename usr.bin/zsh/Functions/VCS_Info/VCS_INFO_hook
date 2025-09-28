### vim:ft=zsh:foldmethod=marker
## Written by Frank Terbeck <ft@bewatermyfriend.org>
## Distributed under the same BSD-ish license as zsh itself.

local hook static func
local context hook_name
local -i ret
local -a hooks tmp
local -i debug

ret=0
hook_name="$1"
shift
context=":vcs_info:${vcs}+${hook_name}:${usercontext}:${rrn}"
static=":vcs_info-static_hooks:${hook_name}"

zstyle -t "${context}" debug && debug=1 || debug=0
if (( debug )); then
    printf 'VCS_INFO_hook: running hook: "%s"\n' "${hook_name}"
    printf 'VCS_INFO_hook: current context: "%s"\n' "${context}"
    printf 'VCS_INFO_hook: static context: "%s"\n' "${static}"
fi

zstyle -a "${static}" hooks hooks
if (( debug )); then
    printf '+ static hooks: %s\n' "${(j:, :)hooks}"
fi
zstyle -a "${context}" hooks tmp
if (( debug )); then
    printf '+ context hooks: %s\n' "${(j:, :)tmp}"
fi
hooks+=( "${tmp[@]}" )
(( ${#hooks} == 0 )) && return 0

# Protect some internal variables in hooks. The `-g' parameter to
# typeset does *not* make the parameters global here (they are already
# "*-local-export). It prevents typeset from creating *new* *local*
# parameters in this function's scope.
typeset -g -r vcs rrn usercontext maxexports msgs vcs_comm
for hook in ${hooks} ; do
    func="+vi-${hook}"
    if (( ${+functions[$func]} == 0 )); then
        (( debug )) && printf '  + Unknown function: "%s"\n' "${func}"
        continue
    fi
    (( debug )) && printf '  + Running function: "%s"\n' "${func}"
    true
    ${func} "$@"
    case $? in
        (0)
            ;;
        (*)
            break
            ;;
    esac
done
typeset -g +r vcs rrn usercontext maxexports msgs vcs_comm
return $ret
