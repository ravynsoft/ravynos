# Log TCP output.
#
# Argument:  Output filename.
#
# Options:
#   -a    Append.  Otherwise the existing file is truncated without warning.
#	  (N.B.: even if logging was already active to it!)
#   -s    Per-session logs.  Output to <filename>1, <filename>2, etc.
#   -c    Close logging.
#   -n/-N Turn off or on normal output; output only goes to the logfile, if
#         any.  Otherwise, output also appears interactively.  This
#         can be given with -c (or any other option), then no output
#         goes anywhere.  However, input is still handled by the usual
#         mechanisms --- $tcp_lines and $TCP_LINE are still set, hence
#         tcp_expect still works.  Equivalent to (un)setting TCP_SILENT.
#
# With no options and no arguments, print the current configuration.
#
# Per-session logs are raw output, otherwise $TCP_PROMPT is prepended
# to each line.

emulate -L zsh
setopt cbases extendedglob

local opt append sess close
integer activity
while getopts "ascnN" opt; do
  (( activity++ ))
  case $opt in
    # append to existing file
    a) append=1
       ;;
    # per-session
    s) sess=1
       ;;
    # close
    c) close=1
       ;;
    # turn off interactive output
    n) TCP_SILENT=1
       ;;
    # turn on interactive output
    N) unset TCP_SILENT
       ;;
    # incorrect option
    \?) return 1
	;;
    # correct option I forgot about
    *) print "$0: option -$opt not handled, oops." >&2
       return 1
       ;;
  esac
done
(( OPTIND > 1 )) && shift $(( OPTIND - 1)) 

if [[ -n $close ]]; then
  if (( $# )); then
    print "$0: too many arguments for -c" >&2
    return 1
  fi
  unset TCP_LOG TCP_LOG_SESS
  return 0
fi

if (( $# == 0 && ! activity )); then
  print "\
Per-session log: ${TCP_LOG_SESS:-<none>}
Overall log:     ${TCP_LOG:-<none>}
Silent?          ${${TCP_SILENT:+yes}:-no}"
  return 0
fi

if (( $# != 1 )); then
  print "$0: wrong number of arguments" >&2
  return 1
fi

if [[ -n $sess ]]; then
  typeset -g TCP_LOG_SESS=$1
  if [[ -z $append ]]; then
    local sesslogs
    integer i
    sesslogs=(${TCP_LOG_SESS}*(N))
    # yes, i know i can do this with multios
    for (( i = 1; i <= $#sesslogs; i++ )); do
      : >$sesslogs[$i]
    done
  fi
else
  typeset -g TCP_LOG=$1
  [[ -z $append ]] && : >$TCP_LOG
fi

return 0
