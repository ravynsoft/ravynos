# Helper function for reading input from a TCP connection.
# Actually, the input doesn't need to be a TCP connection at all, it
# is simply an input file descriptor.  However, it must be contained
# in ${tcp_by_fd[$TCP_SESS]}.  This is set by tcp_open, but may be
# set by hand.  (Note, however, the blocking/timeout behaviour is usually
# not implemented for reading from regular files.)
#
# The default behaviour is simply to read any single available line from
# the input fd and print it.  If a line is read, it is stored in the
# parameter $TCP_LINE; this always contains the last line successfully
# read.  Any chunk of lines read in are stored in the array $tcp_lines;
# this always contains a complete list of all lines read in by a single
# execution of this function and hence may be empty.  The fd corresponding
# to $TCP_LINE is stored in $TCP_LINE_FD (this can be turned into a
# session by looking up in $tcp_by_fd).
#
# Printed lines are preceded by $TCP_PROMPT.  This may contain two
# percent escapes: %s for the current session, %f for the current file
# descriptor.  The default is `T[%s]:'.  The prompt is not printed
# to per-session logs where the source is unambiguous.
#
# The function returns 0 if a read succeeded, even if (using -d) a
# subsequent read failed.
#
# The behaviour is modified by the following options.
#
# -a     Read from all fds, not just the one given by TCP_SESS.
#
# -b	 The first read blocks until some data is available for reading.
#
# -d     Drain all pending input; loop until no data is available.
#
# -l sess1,sess2,...
#        Gives a list of sessions to read on.  Equivalent to
#        -u ${tcp_by_name[sess1]} -u ${tcp_by_name[sess2]} ...
#	 Multiple -l options also work.
#
# -q     Quiet; if $TCP_SESS is not set, just return 1, but don't print
#        an error message.
#
# -s sess
#        Gives a single session; the option may be repeated.
#
# -t TO  On each read (the only read unless -d was also given), time out
#        if nothing was available after TO seconds (may be floating point).
#        Otherwise,  the function will return immediately when no data is
#	 available.
#
#        If combined with -b, the function will always wait for the
#        first data to become available; hence this is not useful unless
#        -d is specified along with -b, in which case the timeout applies
#        to data after the first line.
# -u fd  Read from fd instead of the default session; may be repeated for
#        multiple sessions.  Can be a comma-separated list, too.
# -T TO  This sets an overall timeout, again in seconds.

emulate -L zsh
setopt extendedglob cbases
# set -x

zmodload -i zsh/mathfunc

local opt drain line quiet block read_fd all sess key val noprint
local -A read_fds
read_fds=()
float timeout timeout_all endtime
integer stat

while getopts "abdl:qs:t:T:u:" opt; do
  case $opt in
    # Read all sessions.
    (a) all=1
	;;
    # Block until we receive something.
    (b) block=1
	;;
    # Drain all pending input.
    (d) drain=1
	;;
    (l) for sess in ${(s.,.)OPTARG}; do
	  read_fd=${tcp_by_name[$sess]}
	  if [[ -z $read_fd ]]; then
	    print "$0: no such session: $sess" >&2
	    return 1
	  fi
	  read_fds[$read_fd]=1
	done
	;;

    # Don't print an error message if there is no TCP connection,
    # just return 1.
    (q) quiet=1
	;;
    # Add a single session to the list
    (s) read_fd=${tcp_by_name[$OPTARG]}
        if [[ -z $read_fd ]]; then
	    print "$0: no such session: $sess" >&2
	    return 1
	fi
	read_fds[$read_fd]=1
        ;;
    # Per-read timeout: wait this many seconds before
    # each read.
    (t) timeout=$OPTARG
        [[ -n $TCP_READ_DEBUG ]] && print "Timeout per-operations is $timeout" >&2
	;;
    # Overall timeout: return after this many seconds.
    (T) timeout_all=$OPTARG
	;;
    # Read from given fd(s).
    (u) for read_fd in ${(s.,.)OPTARG}; do
	  if [[ $read_fd != (0x[[:xdigit:]]##|[[:digit:]]##) ]]; then
	    print "Bad fd in $OPTARG" >&2
	    return 1
	  fi
	  read_fds[$((read_fd))]=1
	done
	;;
    (*) [[ $opt != \? ]] && print "Unhandled option, complain: $opt" >&2
	return 1
       ;;
  esac
done

if [[ -n $all ]]; then
  read_fds=(${(kv)tcp_by_fd})
elif (( ! $#read_fds )); then
  if [[ -z $TCP_SESS ]]; then
    [[ -z $quiet ]] && print "No tcp connection open." >&2
    return 1
  elif [[ -z $tcp_by_name[$TCP_SESS] ]]; then
    print "TCP session $TCP_SESS has gorn!" >&2
    return 1
  fi
  read_fds[$tcp_by_name[$TCP_SESS]]=1
fi

typeset -ga tcp_lines
tcp_lines=()

local helper_stat=2 skip tpat reply REPLY
float newtimeout

if [[ ${(t)SECONDS} != float* ]]; then
  # If called from another function, don't override
  typeset -F TCP_SECONDS_START=$SECONDS
  # Get extra accuracy by making SECONDS floating point locally
  typeset -F SECONDS
fi

if (( timeout_all )); then
  (( endtime = SECONDS + timeout_all ))
fi

zmodload -i zsh/zselect

if [[ -n $block ]]; then
  if (( timeout_all )); then
    # zselect -t uses 100ths of a second
    zselect -t $(( int(100*timeout_all + 0.5) )) ${(k)read_fds} || 
      return $helper_stat
  else
    zselect ${(k)read_fds} || return $helper_stat
  fi
fi

while (( ${#read_fds} )); do
  if [[ -n $block ]]; then
    # We already have data waiting this time through.
    unset block
  else
    if (( timeout_all )); then
      (( (newtimeout = endtime - SECONDS) <= 0 )) && return 2
      if (( ! timeout || newtimeout < timeout )); then
	(( timeout = newtimeout ))
      fi
    fi
    if (( timeout )); then
      if [[ -n $TCP_READ_DEBUG ]]; then
	print "[tcp_read: selecting timeout $timeout on ${(k)read_fds}]" >&2
      fi
      zselect -t $(( int(timeout*100 + 0.5) )) ${(k)read_fds} ||
        return $helper_stat
    else
      if [[ -n $TCP_READ_DEBUG ]]; then
	print "[tcp_read: selecting no timeout on ${(k)read_fds}]" >&2
      fi
      zselect -t 0 ${(k)read_fds} || return $helper_stat
    fi
  fi
  if [[ -n $TCP_READ_DEBUG ]]; then
    print "[tcp_read: returned fds ${reply}]" >&2
  fi
  for read_fd in ${reply[2,-1]}; do
    if ! read -u $read_fd -r line; then
      unset "read_fds[$read_fd]"
      stat=1
      continue
    fi

    helper_stat=0
    sess=${tcp_by_fd[$read_fd]}

    # Handle user-defined triggers
    noprint=${TCP_SILENT:+-q}
    if (( ${+tcp_on_read} )); then
      # Call the function given in the key for each matching value.
      # It is this way round because function names must be
      # unique, while patterns do not need to be.  Furthermore,
      # this keeps the use of subscripting under control.
      for key val in ${(kv)tcp_on_read}; do
	if [[ $line = ${~val} ]]; then
	  $key "$sess" "$line" || noprint=-q
	fi
      done
    fi

    tcp_output -P "${TCP_PROMPT=<-[%s] }" -S $sess -F $read_fd \
        $noprint -- "$line"
    # REPLY is now set to the line with an appropriate prompt.
    tcp_lines+=($REPLY)
    typeset -g TCP_LINE="$REPLY" TCP_LINE_FD="$read_fd"

    # Only handle one line from one device at a time unless draining.
    [[ -z $drain ]] && return $stat
  done
done

return $stat
