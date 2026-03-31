local line fd=$1 sess=${tcp_by_fd[$1]}
local TCP_HANDLER_ACTIVE=1
if [[ -n $sess ]]
then
  local TCP_INVALIDATE_ZLE
  if (( $# > 2 )); then
    zle -I
    ## debugging only
    # print "Flags on the play:" ${argv[3,-1]}
  else
    TCP_INVALIDATE_ZLE=1
  fi
  if ! tcp_read -d -u $fd; then
    if (( ${+functions[tcp_on_awol]} )); then
      tcp_on_awol $sess $fd
      (( $? == 100 )) || return $?
    fi
    [[ -n $TCP_INVALIDATE_ZLE ]] && zle -I
    print "[TCP fd $fd (session $sess) gone awol; removing from poll list]" >& 2
    zle -F $fd
    return 1
  fi
  return 0
else
  zle -I
  # Handle fds not in the TCP set similarly.
  # This does the drain thing, to try and get as much data out as possible.
  if ! read -u $fd line; then
    print "[Reading on $fd failed; removing from poll list]" >& 2
    zle -F $fd
    return 1
  fi
  line="fd$fd:$line"
  local newline
  while read -u $fd -t newline; do
    line="${line}
fd$fd:$newline"
  done
fi
print -r - $line
