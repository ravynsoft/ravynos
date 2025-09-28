emulate -L zsh
setopt extendedglob cbases

local opt quiet all sess fd nonewline cat line
local -a sessions write_fds
integer mystat

while getopts "acl:nqs:" opt; do
    case $opt in
	(a) all=1
	    ;;
        (c) cat=1
	    ;;
	(n) nonewline=-n
	    ;;
	(q) quiet=1
	    ;;
	(l) for sess in ${(s.,.)OPTARG}; do
	        if [[ -z ${tcp_by_name[$sess]} ]]; then
		    print "$0: no such session: $sess" >&2
		    return 1
		fi
		sessions+=($sess)
	    done
	    ;;
	(s) if [[ -z $tcp_by_name[$OPTARG] ]]; then
                print "No such session: $OPTARG" >&2
		return 1
	    fi
	    sessions+=($OPTARG)
	    ;;
	(*) [[ $opt != '?' ]] && print "Unhandled option, complain: $opt" >&2
            return 1
	    ;;
    esac
done
(( OPTIND > 1 )) && shift $(( OPTIND - 1 ))

if [[ -n $all ]]; then
    sessions=(${(k)tcp_by_name})
elif (( ! ${#sessions} )); then
    sessions=($TCP_SESS)
fi
if (( ! $#sessions )); then
    if [[ -z $quiet ]]; then
	print "No current TCP session open." >&2
    fi
    return 1
fi

# Writing on a TCP connection closed by the remote end can cause SIGPIPE.
# The following test is reasonably robust, though in principle we can
# mistake a SIGPIPE owing to another fd.  That doesn't seem like a big worry.
# `emulate -L zsh' will already have set localtraps.
local TCP_FD_CLOSED
trap 'TCP_FD_CLOSED=1' PIPE

local TCP_SESS

while true; do
  if [[ -n $cat ]]; then
    read -r line || break
  else
    line="$*"
  fi
  for TCP_SESS in $sessions; do
    fd=${tcp_by_name[$TCP_SESS]}
    if [[ -z $fd ]]; then
      print "No such session: $TCP_SESS" >&2
      mystat=1
      continue
    fi
    print -u $fd $nonewline -r -- $line
    if [[ $? -ne 0 || -n $TCP_FD_CLOSED ]]; then
      print "Session ${TCP_SESS}: fd $fd unusable." >&2
      unset TCP_FD_CLOSED
      mystat=1
      continue
    fi
    if [[ -n $TCP_OUTPUT ]]; then
      tcp_output -P "$TCP_OUTPUT" -S $TCP_SESS -F $fd -q "${(j. .)*}"
    fi
  done
  [[ -z $cat ]] && break
done

return $mystat
