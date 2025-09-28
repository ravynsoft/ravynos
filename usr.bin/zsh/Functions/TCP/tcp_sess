# try to disguise parameters from the eval'd command in case it's a function.
integer __myfd=1

if [[ -n $1 ]]; then
  if [[ -z $tcp_by_name[$1] ]]; then
    print no such session: $1
    __myfd=2
  elif [[ -n $2 ]]; then
    local TCP_SESS=$1
    shift
    # A bit tricky: make sure the first argument gets re-evaluated,
    # so as to get aliases etc. to work, but make sure the remainder
    # don't, so as not to bugger up quoting.  This ought to work the
    # vast majority of the time, anyway.
    local __cmd=$1
    shift
    eval $__cmd \$\*
    return
  else
    typeset -g TCP_SESS=$1
    return 0;
  fi
fi

# Print out the list of sessions, first the number, than the corresponding
# file descriptor.  The current session, if any, is marked with an asterisk.
local cur name fd
for name in ${(ko)tcp_by_name}; do
  fd=${tcp_by_name[$name]}
  # mark current session with an asterisk
  if [[ ${TCP_SESS} = $name ]]; then
    cur=" *"
  else
    cur=
  fi
  print -u $__myfd "sess:$name; fd:$fd$cur"
done

return $(( __myfd - 1 ))
