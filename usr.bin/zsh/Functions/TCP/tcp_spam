# SPAM is a registered trademark of Hormel Foods Corporation.
#
# -a all connections, override $tcp_spam_list and $tcp_no_spam_list.
#    If not given and tcp_spam_list is set to a list of sessions,
#    only those will be spammed.  If tcp_no_spam_list is set, those
#    will (also) be excluded from spamming.
# -e use `eval' to run the command list instead of executing as
#    a normal command line.
# -l sess1,sess2    give comma separated list of sessions to spam
# -r reverse, spam in opposite order (default is alphabetic, -r means
#    omegapsiic).  Note tcp_spam_list is not sorted (but may be reversed).
# -t transmit, send data to slave rather than executing command for eac
#    session.
# -v verbose, list session being spammed in turn
#
# If the function tcp_on_spam is defined, it is called for each link
# with the first argument set to the session name, and the remainder the
# command line to be executed.  If it sets the parameter REPLY to `done',
# the command line will not then be executed by tcp_spam, else it will.

emulate -L zsh
setopt extendedglob

local cursess=$TCP_SESS sessstr
local TCP_SESS cmd opt verbose reverse sesslist transmit all eval
local match mbegin mend REPLY
local -a sessions

while getopts "ael:rtv" opt; do
    case $opt in
	(a) all=1
	    ;;
	(e) eval=1
	    ;;
	(l) sessions+=(${(s.,.)OPTARG})
	    ;;
	(r) reverse=1
	    ;;
	(s) sessions+=($OPTARG)
	    ;;
	(t) transmit=-t
	    ;;
	(v) verbose=1
	    ;;
	(*) [[ $opt != '?' ]] && print "Option $opt not handled." >&2
	    print "Sorry, spam's off." >&2
	    return 1
	    ;;
    esac
done
(( OPTIND > 1 )) && shift $(( OPTIND - 1 ))

local name
if [[ -n $all ]]; then
    sessions=(${(ko)tcp_by_name})
elif (( ! ${#sessions} )); then
  if (( ${#tcp_spam_list} )); then
    sessions=($tcp_spam_list)
  else
    sessions=(${(ko)tcp_by_name})
  fi
  if (( ${#tcp_no_spam_list} )); then
    for name in ${tcp_no_spam_list}; do
      sessions=(${sessions:#$name})
    done
  fi
fi

if [[ -n $reverse ]]; then
  local tmp
  integer i
  for (( i = 1; i <= ${#sessions}/2; i++ )); do
    tmp=${sessions[i]}
    sessions[i]=${sessions[-i]}
    sessions[-i]=$tmp
  done
fi

if (( ! ${#sessions} )); then
  print "No connections to spam." >&2
  return 1
fi
if (( ! $# )); then
  print "No commands given." >&2
  return 1
fi

if [[ -n $transmit ]]; then
  cmd=tcp_send
elif [[ -z $eval ]]; then
  cmd=$1
  shift
fi

: ${TCP_PROMPT:=T[%s]:}

for TCP_SESS in $sessions; do
  REPLY=
  if (( ${+functions[tcp_on_spam]} )); then
    tcp_on_spam $TCP_SESS $cmd $*
    [[ $REPLY = done ]] && continue
  fi
  if [[ -n $verbose ]]; then
      if [[ $TCP_SESS = $cursess ]]; then
	  sessstr="c:1"
      else
	  sessstr="c:0"
      fi
      zformat -f REPLY $TCP_PROMPT "s:$TCP_SESS" \
	  "f:${tcp_by_name[$TCP_SESS]}" $sessstr && print -r $REPLY
  fi
  if [[ -n $eval ]]; then
      eval $*
  else
      eval $cmd '$*'
  fi
done
