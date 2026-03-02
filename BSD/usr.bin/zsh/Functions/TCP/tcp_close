# Usage:
#   tcp_close [-q] [ -a | session ... ]
# -a means all sessions.
# -n means don't close a fake session's fd.
# -q means quiet.
#
# Accepts the -s and -l arguments for consistency with other functions,
# but there is no particular gain in using them
emulate -L zsh
setopt extendedglob cbases

local all quiet opt alias noclose
local -a sessnames

while getopts "aql:ns:" opt; do
    case $opt in
	(a) all=1
	    ;;
	(q) quiet=1
	    ;;
	(l) sessnames+=(${(s.,.)OPTARG})
	    ;;
	(n) noclose=1
	    ;;
	(s) sessnames+=($OPTARG)
	    ;;
	(*) return 1
	    ;;
    esac
done

(( OPTIND > 1 )) && shift $(( OPTIND - 1))

if [[ -n $all ]]; then
    if (( $# )); then
	print "Usage: $0 [ -q ] [ -a | [ session ... ] ]" >&2
	return 1
    fi
    sessnames=(${(k)tcp_by_name})
    if (( ! ${#sessnames} )); then
	[[ -z $quiet ]] && print "No TCP sessions open." >&2
	return 1
    fi
fi

sessnames+=($*)

if (( ! ${#sessnames} )); then
    sessnames+=($TCP_SESS)
fi

if (( ! ${#sessnames} )); then
    [[ -z $quiet ]] && print "No current TCP session." >&2
    return 1
fi

local tcp_sess fd
integer stat curstat

# Check to see if the fd is opened for a TCP session, or was opened
# to a pre-existing fd.  We could remember this from tcp_open.
local -A ztcp_fds
local line match mbegin mend

if zmodload -e zsh/net/tcp; then
    ztcp | while read line; do
        if [[ $line = (#b)*fd\ ([0-9]##) ]]; then
	    ztcp_fds[$match[1]]=1
	fi
    done
fi

for tcp_sess in $sessnames; do
    curstat=0
    fd=${tcp_by_name[$tcp_sess]}
    if [[ -z $fd ]]; then
	print "No TCP session $tcp_sess!" >&2
	stat=1
	continue
    fi
    # We need the base name if this is an alias.
    tcp_sess=${tcp_by_fd[$fd]}
    if [[ -z $tcp_sess ]]; then
	if [[ -z $quiet ]]; then
	    print "Aaargh!  Session for fd $fd has disappeared!" >&2
	fi
	stat=1
	continue
    fi

    if [[ ${+tcp_aliases} -ne 0 && -n ${tcp_aliases[$fd]} ]]; then
	for alias in ${=tcp_aliases[$fd]}; do
	    if (( ${+functions[tcp_on_unalias]} )); then
		tcp_on_unalias $alias $fd
	    fi
	    unset "tcp_by_name[$alias]"
	done
	unset "tcp_aliases[$fd]"
    fi

    # Don't return just because the zle handler couldn't be uninstalled...
    if [[ -o zle ]]; then
	zle -F $fd || print "[Ignoring...]" >&2
    fi

    if [[ -n $ztcp_fds[$fd] ]]; then
        # It's a ztcp session.
	if ! ztcp -c $fd; then
	    stat=1
	    curstat=1
	fi
    elif [[ -z $noclose ]]; then
        # It's not, just close it normally.
        # Careful: syntax for closing fd's is quite strict.
	if [[ ${#fd} -gt 1 ]]; then
	    [[ -z $quiet ]] && print "Can't close fd $fd; will leave open." >&2
	else
	    eval "exec $fd>&-"
	fi
    fi

    unset "tcp_by_name[$tcp_sess]"
    unset "tcp_by_fd[$fd]"
    if [[ -z $quiet && $curstat -eq 0 ]]; then
	print "Session $tcp_sess successfully closed."
    fi
    [[ $tcp_sess = $TCP_SESS ]] && unset TCP_SESS

    if (( ${+functions[tcp_on_close]} )); then
	tcp_on_close $tcp_sess $fd
    fi
done

return $stat
