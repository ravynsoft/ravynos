# Open a TCP session, add it to the list, handle it with zle if that's running.
# Unless using -a, -f, -l or -s, first two arguments are host and port.
#
# Remaining argument, if any, is the name of the session, which mustn't
# clash with an existing one.  If none is given, the number of the
# connection is used (i.e. first connection is 1, etc.), or the first
# available integer if that is already in use.
#
# Session names, whether provided on the command line or in the
# .ztcp_sessions file should not be `clever'.  A clever name is one
# with characters that won't work.  This includes whitespace and an
# inconsistent set of punctuation characters.  If in doubt, stick
# to alphanumeric, underscore and non-initial hyphen.
#
# -a fd   Accept a connection on fd and make that the session.
#         This will block until a successful incoming connection is received.
#
#         fd is probably a value returned by ztcp -l; no front-end
#         is currently provided for that but it should simply be
#         a matter of calling `ztcp -l port' and storing $REPLY, then
#         closing the listened port with `ztcp -c $stored_fd'.
#
# -f fd   `Fake' tcp connection on the given file descriptor.  This
#         could be, for example, a file descriptor already opened to
#         a named pipe.  It should not be a regular file, however.
#         Note that it is not a good idea for two different sessions
#         to be attempting to read from the same named pipe, so if
#         both ends of the pipe are to be handled by zsh, at least
#         one should use the `-z' option.
#
# -l sesslist
# -s sessname
#         Open by session name or comma separated list; either may
#         be repeated as many times as necessary.  The session must be
#	  listed in the file ${ZDOTDIR:-$HOME}/.ztcp_sessions.  Lines in
#	  this file look exactly like a tcp_open command line except the
#	  session name is at the start, for example
#           sess1 pwspc 2811
#         has the effect of
#           tcp_open pwspc 2811 sess1
#         Remaining arguments (other than options) to tcp_open are
#         not allowed.  Options in .ztcp_sessions are not handled.
#	  The file must be edited by hand.
#
# -z      Don't install a zle handler for reading on the file descriptor.
#	  Otherwise, if zle is enabled, the file descriptor will
#         be tested while at the shell prompt and any input automatically
#         printed in the same way as job control notification.
#
# If a session is successfully opened, and if the function `tcp_on_open'
# exists, it is run with the arguments session_name, session_fd.

emulate -L zsh
setopt extendedglob cbases

# Global set up for TCP function suite.

zmodload -i zsh/net/tcp || return 1
zmodload -i zsh/zutil
autoload -Uz tcp_alias tcp_close tcp_command tcp_expect tcp_fd_handler
autoload -Uz tcp_log tcp_output tcp_proxy tcp_read tcp_rename tcp_send
autoload -Uz tcp_sess tcp_spam tcp_talk tcp_wait tcp_point tcp_shoot

# TCP_SECONDS_START is only set if we override TCP_SECONDS locally,
# so provide a global value for convenience.  Should probably always be 0.
(( ${+TCP_SECONDS_START} )) || typeset -gF TCP_SECONDS_START

# Processing for new connection.

local opt accept fake nozle sessfile sess quiet
local -a sessnames sessargs
integer stat

while getopts "a:f:l:qs:z" opt; do
  case $opt in
    (a) accept=$OPTARG
    if [[ $accept != [[:digit:]]## ]]; then
      print "option -a takes a file descriptor" >&2
      return 1
    fi
    ;;
    (f) fake=$OPTARG
    if [[ $fake != [[:digit:]]## ]]; then
      print "option -f takes a file descriptor" >&2
      return 1
    fi
    ;;
    (l) sessnames+=(${(s.,.)OPTARG})
    ;;
    (q) quiet=1
    ;;
    (s) sessnames+=($OPTARG)
    ;;
    (z) nozle=1
    ;;
    (*) return 1
    ;;
  esac
done
(( OPTIND > 1 )) && shift $(( OPTIND - 1 ))

(( ${+tcp_by_fd} ))   || typeset -gA tcp_by_fd
(( ${+tcp_by_name} )) || typeset -gA tcp_by_name 
typeset -A sessassoc

if (( ${#sessnames} )); then
  if [[ $# -ne 0 || -n $accept || -n $fake ]]; then
    print "Incompatible arguments with \`-s' option." >&2
    return 1
  fi
  for sess in ${sessnames}; do
    sessassoc[$sess]=
  done

  sessfile=${ZDOTDIR:-$HOME}/.ztcp_sessions
  if [[ ! -r $sessfile ]]; then
    print "No session file: $sessfile" >&2
    return 1
  fi
  while read -A sessargs; do
    [[ ${sessargs[1]} = '#'* ]] && continue
    if ((  ${+sessassoc[${sessargs[1]}]} )); then
      sessassoc[${sessargs[1]}]="${sessargs[2,-1]}"
    fi
  done < $sessfile
  for sess in ${sessnames}; do
    if [[ -z $sessassoc[$sess] ]]; then
      print "Couldn't find session $sess in $sessfile." >&2
      return 1
    fi
  done
else
  if [[ -z $accept && -z $fake ]]; then
    if (( $# < 2 )); then
      set -- wrong number of arguments
    else
      host=$1 port=$2
      shift $(( $# > 1 ? 2 : 1 ))
    fi
  fi
  if [[ -n $1 ]]; then
    sessnames=($1)
    shift
  else
    sessnames=($(( ${#tcp_by_fd} + 1 )))
    while [[ -n $tcp_by_name[$sessnames[1]] ]]; do
      (( sessnames[1]++ ))
    done
  fi
  sessassoc[$sessnames[1]]="$host $port"
fi

if (( $# )); then
  print "Usage: $0 [-z] [-a fd | -f fd | host port [ session ] ]
       $0 [-z] [ -s session | -l sesslist ] ..." >&2
  return 1
fi

local REPLY fd
for sess in $sessnames; do
  if [[ -n $tcp_by_name[$sess] ]]; then
    print "Session \`$sess' already exists." >&2
    return 1
  fi

  sessargs=()
  if [[ -n $fake ]]; then
    fd=$fake;
  else
    if [[ -n $accept ]]; then
      ztcp -a $accept || return 1
    else
      sessargs=(${=sessassoc[$sess]})
      ztcp $sessargs || return 1
    fi
    fd=$REPLY
  fi

  tcp_by_fd[$fd]=$sess
  tcp_by_name[$sess]=$fd

  [[ -o zle && -z $nozle ]] && zle -F $fd tcp_fd_handler

  # needed for new completion system, so I'm not too sanguine
  # about requiring this here...
  if zmodload -i zsh/parameter; then
    if (( ${+functions[tcp_on_open]} )); then
      if ! tcp_on_open $sess $fd; then
	if [[ -z $quiet ]]; then
	  if (( ${#sessargs} )); then
	    print "Session $sess" \
"(host $sessargs[1], port $sessargs[2] fd $fd): tcp_on_open FAILED."
	  else
	    print "Session $sess (fd $fd) tcp_on_open FAILED."
	  fi
	  tcp_close -- $sess
	else
	  tcp_close -q -- $sess
	fi
	stat=1
	continue
      fi
    fi
  fi

  if [[ -z $quiet ]]; then
    if (( ${#sessargs} )); then
      print "Session $sess" \
"(host $sessargs[1], port $sessargs[2] fd $fd) opened OK."
    else
      print "Session $sess (fd $fd) opened OK."
    fi
  fi
done

if [[ -z $TCP_SESS || -z $tcp_by_name[$TCP_SESS] ]]; then
  # careful in case we closed it again...
  if [[ -n $tcp_by_name[$sessnames[1]] ]]; then
    [[ -z $quiet ]] && print "Setting default TCP session $sessnames[1]"
    typeset -g TCP_SESS=$sessnames[1]
  fi
fi

return $stat
