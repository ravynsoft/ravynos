# Rename session OLD (defaults to current session) to session NEW.
# Does not handle aliases; use tcp_alias for all alias redefinitions.

local old new

if (( $# == 1 )); then
  old=$TCP_SESS
  new=$1
elif (( $# == 2 )); then
  old=$1
  new=$2
else
  print "Usage: $0 OLD NEW" >&2
  return 1
fi

local fd=$tcp_by_name[$old]
if [[ -z $fd ]]; then
  print "No such session: $old" >&2
  return 1
fi
if [[ -n $tcp_by_name[$new] ]]; then
  print "Session $new already exists." >&2
  return 1
fi
# Can't rename an alias
if [[ $tcp_by_fd[$fd] != $old ]]; then
  print "Use tcp_alias to redefine an alias." >&2
  return 1
fi

tcp_by_name[$new]=$fd
unset "tcp_by_name[$old]"

tcp_by_fd[$fd]=$new

[[ $TCP_SESS = $old ]] && TCP_SESS=$new

if zmodload -i zsh/parameter; then
  if (( ${+functions[tcp_on_rename]} )); then
    tcp_on_rename $new $fd $old
  fi
fi
