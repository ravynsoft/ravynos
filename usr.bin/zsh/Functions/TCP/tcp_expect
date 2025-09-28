# Expect one of a series of regular expressions from $TCP_SESS.
# Can make backreferences to be handled by $match.  Returns 0 for
# successful match, 1 for error, 2 for timeout.
#
# This function has no facility for conditionally calling code based
# the regular expression found.  This should be done in the calling code
# by testing $TCP_LINE, which contains the line which matched the
# regular expression.  The complete set of lines read while waiting for
# this line is available in the array $tcp_expect_lines (including $TCP_LINE
# itself which will be the final element).  Alternatively, use -p pind
# which sets $pind to the index of the pattern which matched.  It
# will be set to 0 otherwise.
#
# Many of the options are passed straight down to tcp_read.
#
# Options:
#   -a     Run tcp_expect across all sessions; the first pattern matched
#          from any session is used.  The TCP output prompt can be
#          used to decide which session matched.
#   -l list
#          Comma-separated list of sessions as for tcp_read.
#   -p pv  If the Nth of a series of patterns matches, set the parameter
#          whose name is given by $pv to N; in the case of a timeout,
#          set it to -1; otherwise (unless the function exited prematurely),
#	   set it to 0.
#	   To avoid namespace clashes, the parameter's name must
#	   not begin with `_expect'.
#   -P pv  This is similar to -p, however in this case the
#          arguments to tcp_expect following the options are expected
#          to start with a prefix "<tag>:".  The parameter $pv is
#          then set to the value "<tag>" rather than the numeric
#          index of the parameter.  The string "timeout" is used
#          as the tag for a timeout specified by -t and -T and
#          on a failed match the variable is set to the empty string.
#          It is not an error for multiple arguments to have
#          the same tag or to use a reserved value of the tag.
#   -q     Quiet, passed down to tcp_read.  Bad option and argument
#          usage is always reported.
#   -s sess
#          Expect from session sess.  May be repeated for multiple sessions.
#   -t to  Timeout in seconds (may be floating point) per read operation.
#          tcp_expect will only time out if every read operation takes longer
#          than to
#   -T TO  Overall timeout; tcp_expect will time out if the overall operation
#          takes longer than this many seconds.
emulate -L zsh
setopt extendedglob

if [[ ${(t)SECONDS} != float* ]]; then
    # If called from another function, use that
    typeset -F TCP_SECONDS_START=$SECONDS
    # Get extra accuracy by making SECONDS floating point locally
    typeset -F SECONDS
fi

# Variables are all named _expect_* to avoid problems with the -p param.
local _expect_opt _expect_pvar _expect_state _expect_arg _expect_ind
local -a _expect_read_args
float _expect_to1 _expect_to_all _expect_to _expect_new_to
integer _expect_i _expect_stat _expect_states

while getopts "al:p:P:qs:t:T:" _expect_opt; do
  case $_expect_opt in
    (a) _expect_read_args+=(-a)
	;;
    (l) _expect_read_args+=(-l $OPTARG)
        ;;
    ([pP]) _expect_pvar=$OPTARG
	if [[ $_expect_pvar != [a-zA-Z_][a-zA-Z_0-9]# ]]; then
	  print "invalid parameter name: $_expect_pvar" >&2
	  return 1
	fi
	if [[ $_expect_pvar = _expect* ]]; then
	  print "$0: parameter names staring \`_expect' are reserved."
	  return 1
	fi
	if [[ $_expect_opt = "P" ]]; then
	  eval "$_expect_pvar=0"
	  _expect_states=1
	else
	  eval "$_expect_pvar="
	fi
	;;
    (q) _expect_read_args+=(-q)
        ;;
    (s) _expect_read_args+=(-s $OPTARG)
        ;;
    (t) _expect_to1=$OPTARG
	;;
    (T) _expect_to_all=$(( SECONDS + $OPTARG ))
        ;;
    (\?) return 1
	 ;;
    (*) print Unhandled option $_expect_opt, complain >&2
	return 1
	;;
  esac
done
(( OPTIND > 1 )) && shift $(( OPTIND - 1 ))

typeset -ga tcp_expect_lines
tcp_expect_lines=()
while true; do
  if (( _expect_to_all || _expect_to1 )); then
    _expect_to=0
    (( _expect_to1 )) && (( _expect_to = _expect_to1 ))
    if (( _expect_to_all )); then
      # overall timeout, see if it has already triggered
      if (( (_expect_new_to = (_expect_to_all - SECONDS)) <= 0 )); then
	[[ -n $_expect_pvar ]] && eval "$_expect_pvar=-1"
	return 2
      fi
      if (( _expect_to <= 0 || _expect_new_to < _expect_to )); then
	_expect_to=$_expect_new_to
      fi
    fi
    tcp_read $_expect_read_args -t $_expect_to
    _expect_stat=$?
  else
    tcp_read $_expect_read_args -b
    _expect_stat=$?
  fi
  if (( _expect_stat )); then
    [[ -n $_expect_pvar ]] && eval "$_expect_pvar=-1"
    return $_expect_stat
  fi
  tcp_expect_lines+=($TCP_LINE)
  for (( _expect_i = 1; _expect_i <= $#; _expect_i++ )); do
    if [[ _expect_states -ne 0 && $argv[_expect_i] = (#b)([^:]#):(*) ]]; then
      _expect_ind=$match[1]
      _expect_arg=$match[2]
    else
      _expect_ind=$_expect_i
      _expect_arg=$argv[_expect_i]
    fi
    if [[ "$TCP_LINE" = ${~_expect_arg} ]]; then
      [[ -n $_expect_pvar ]] && eval "$_expect_pvar=\$_expect_ind"
      return 0
    fi
  done
done
