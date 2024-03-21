emulate -L zsh
setopt extendedglob

local opt tprompt sess read_fd tpat quiet cursess

while getopts "F:P:qS:" opt; do
  case $opt in
    (F) read_fd=$OPTARG
	;;
    (P) tprompt=$OPTARG
	;;
    (q) quiet=1
	;;
    (S) sess=$OPTARG
	;;
    (*) [[ $opt != \? ]] && print -r "Can't handle option $opt" >&2
	return 1
	;;
  esac
done
(( OPTIND > 1 )) && shift $(( OPTIND - 1 ))

# Per-session logs don't have the session discriminator in front.
if [[ -n $TCP_LOG_SESS ]]; then
  print -r -- "$*" >>${TCP_LOG_SESS}.$sess
fi
# Always add the TCP prompt.  We used only to do this with
# multiple sessions, but it seems always to be useful to know
# where data is coming from; also, it allows more predictable
# behaviour in tcp_expect.
if [[ -n $tprompt ]]; then
  if [[ $sess = $TCP_SESS ]]; then
      cursess="c:1"
  else
      cursess="c:0"
  fi
  zformat -f REPLY $tprompt "s:$sess" "f:$read_fd" $cursess
  if [[ $REPLY = %P* ]]; then
    REPLY=${(%)${REPLY##%P}}
  fi
  # We will pass this back up.
  REPLY="$REPLY$*"
else
  REPLY="$*"
fi
if [[ -n $TCP_LOG ]]; then
  print -r -- $REPLY >>${TCP_LOG}
fi

if [[ -z $quiet ]]; then
  local skip=
  if [[ ${#tcp_filter} -ne 0 ]]; then
    # Allow tcp_filter to be an associative array, though
    # it doesn't *need* to be.
    for tpat in ${(v)tcp_filter}; do
      [[ $REPLY = ${~tpat} ]] && skip=1 && break
    done
  fi
  if [[ -z $skip ]]; then
    # Check flag passed down probably from tcp_fd_handler:
    # if we have output, we are in zle and need to fix the display first.
    # (The shell is supposed to be smart enough that you can replace
    # all the following with
    #   [[ -o zle ]] && zle -I
    # but I haven't dared try it yet.)
    if [[ -n $TCP_INVALIDATE_ZLE ]]; then
      zle -I
      # Only do this the first time.
      unset TCP_INVALIDATE_ZLE
    fi
    print -r -- $REPLY
  fi
fi
