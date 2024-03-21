# Create an alias for a TCP session.
#
# The syntax is similar to the `alias' builtin.  Aliases with a trailing
# `=' are assigned, while those without are listed.
#
# The alias can be used to refer to the session, however any output
# from the session will be shown using information for the base
# session name.  Likewise, any other reference to the session's file
# descriptor will cause the original session name rather than the alias to
# be used.
#
# It is an error to attempt to create an alias for a non-existent session.
# The alias will be removed when the session is closed.
#
# An alias can be reused without the session having to be closed.
# However, a base session name cannot be used as an alias.  If this
# becomes necessary, the base session should be renamed with tcp_rename
# first.
#
# With no arguments, list aliases.
#
# With the option -d, delete the alias.  No value is allowed in this case.
#
# With the option -q (quiet), just return status 1 on failure.  This
# does not apply to bad syntax, which is always reported.  Bad syntax
# includes deleting aliases when supplying a value.

emulate -L zsh
setopt extendedglob cbases

local opt quiet base value alias delete arg match mbegin mend fd array
integer stat index

while getopts "qd" opt; do
  case $opt in
    (q) quiet=1
	;;
    (d) delete=1
	;;
    (*) return 1
	;;
  esac
done
(( OPTIND > 1 )) && shift $(( OPTIND - 1 ))

if (( ! $# )); then
  if (( ${#tcp_aliases} )); then
    for fd value in ${(kv)tcp_aliases}; do
      for alias in ${=value}; do
	print -r - \
	"${alias}: alias for session ${tcp_by_fd[$fd]:-unnamed fd $fd}"
      done
    done
  fi
  return 0
fi

for arg in $*; do
  if [[ $arg = (#b)([^=]##)=(*) ]]; then
    if [[ -n $delete ]]; then
      print "$0: value given with deletion command." >&2
      stat=1
      continue
    fi
    alias=$match[1]
    base=$match[2]
    if [[ -z $base ]]; then
      # hmm, this is very nearly a syntax error...
      [[ -z $quiet ]] && print "$0: empty value for alias $alias" >&2
      stat=1
      continue
    fi
    if [[ ${+tcp_by_name} -eq 0 || -z ${tcp_by_name[$base]} ]]; then
      [[ -z $quiet ]] && print "$0: no base session \`$base' for alias"
      stat=1
      continue
    fi
    if [[ -n ${tcp_by_name[$alias]} ]]; then
      # already exists, OK if this is an alias...
      fd=${tcp_by_name[$alias]}
      array=(${=tcp_aliases[$fd]})
      if [[ -n ${array[(r)$alias]} ]]; then
	# yes, we're OK; delete the old alias.
	unset "tcp_by_name[$alias]"
	index=${array[(i)$alias]}
	array=(${array[1,index-1]} ${array[index+1,-1]})
	if [[ -z "$array" ]]; then
	  unset "tcp_aliases[$fd]"
	else
	  tcp_aliases[$fd]="$array"
	fi
      else
	# oops
	if [[ -z $quiet ]]; then
	  print "$0: \`$alias' is already a session name." >&2
	fi
	stat=1
	continue
      fi
    fi
    (( ! ${+tcp_aliases} )) && typeset -gA tcp_aliases
    fd=${tcp_by_name[$base]}
    if [[ -n ${tcp_aliases[$fd]} ]]; then
      tcp_aliases[$fd]+=" $alias"
    else
      tcp_aliases[$fd]=$alias
    fi
    tcp_by_name[$alias]=$fd
    if zmodload -i zsh/parameter; then
      if (( ${+functions[tcp_on_alias]} )); then
	tcp_on_alias $alias $fd
      fi
    fi
  else
    alias=$arg
    fd=${tcp_by_name[$alias]}
    if [[ -z $fd ]]; then
      print "$0: no such alias \`$alias'" >&2
      stat=1
      continue
    fi
    # OK if this is an alias...
    array=(${=tcp_aliases[$fd]})
    if [[ -n ${array[(r)$alias]} ]]; then
      # yes, we're OK
      if [[ -n $delete ]]; then
	unset "tcp_by_name[$alias]"
	index=${array[(i)$alias]}
	array=(${array[1,index-1]} ${array[index+1,-1]})
	if [[ -z "$array" ]]; then
	  unset "tcp_aliases[$fd]"
	else
	  tcp_aliases[$fd]="$array"
	fi

	if zmodload -i zsh/parameter; then
	  if (( ${+functions[tcp_on_unalias]} )); then
	    tcp_on_unalias $alias $fd
	  fi
	fi
      else
	print -r - \
	  "${alias}: alias for session ${tcp_by_fd[$fd]:-unnamed fd $fd}"
      fi
    else
      # oops
      if [[ -z $quiet ]]; then
	print "$0: \`$alias' is a session name." >&2
      fi
      stat=1
	continue
    fi
  fi
done

return $stat
