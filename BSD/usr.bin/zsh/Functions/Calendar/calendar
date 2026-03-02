emulate -L zsh
setopt extendedglob

local line showline restline REPLY REPLY2 userange nobackup datefmt
local calendar donefile sched newfile warnstr mywarnstr newdate
integer time start stop today ndays y m d next=-1 shown done nodone
integer verbose warntime mywarntime t tcalc tsched i rstat remaining
integer showcount icount repeating repeattime resched showall brief
local -a calendar_entries calendar_addlines
local -a times calopts showprog lockfiles match mbegin mend tmplines
local -A reply

zmodload -i zsh/datetime || return 1
zmodload -i zsh/zutil || return 1
zmodload -F zsh/files b:zf_ln || return 1

autoload -Uz calendar_{add,parse,read,scandate,show,lockfiles}

# Read the calendar file from the calendar-file style
zstyle -s ':datetime:calendar:' calendar-file calendar || calendar=~/calendar
newfile=$calendar.new.$HOST.$$
zstyle -s ':datetime:calendar:' done-file donefile || donefile="$calendar.done"
# Read the programme to show the message from the show-prog style.
zstyle -a ':datetime:calendar:' show-prog showprog ||
  showprog=(calendar_show)
# Amount of time before an event when it should be flagged.
# May be overridden in individual entries
zstyle -s ':datetime:calendar:' warn-time warnstr || warnstr="0:05"
# default to standard ctime date/time format
zstyle -s ':datetime:calendar:' date-format datefmt ||
  datefmt="%a %b %d %H:%M:%S %Z %Y"

if [[ -n $warnstr ]]; then
  if [[ $warnstr = <-> ]]; then
    (( warntime = warnstr ))
  elif ! calendar_scandate -ar $warnstr; then
    print >&2 \
      "warn-time value '$warnstr' not understood; using default 5 minutes"
    warnstr="5 mins"
    (( warntime = 5 * 60 ))
  else
    (( warntime = REPLY ))
  fi
fi

[[ -f $calendar ]] || return 1

# We're not using getopts because we want +... to refer to a
# relative time, not an option, and allow some other additions
# like handling -<->.
integer opti=0
local opt optrest optarg

while [[ ${argv[opti+1]} = -* ]]; do
  (( opti++ ))
  opt=${argv[opti][2]}
  optrest=${argv[opti][3,-1]}
  [[ -z $opt || $opt = - ]] && break
  while [[ -n $opt ]]; do
    case $opt in
      ########################
      # Options with arguments
      ########################
      ([BCnS])
      if [[ -n $optrest ]]; then
	optarg=$optrest
	optrest=
      elif (( opti < $# )); then
	optarg=$argv[++opti]
	optrest=
      else
	print -r "$0: option -$opt requires an argument." >&2
	return 1
      fi
      case $opt in
	(B)
	# Brief, with number of lines to show.
	brief=$optarg
	if (( brief <= 0 )); then
	  print -r "$0: option -$opt requires a positive integer." >&2
	  return 1
	fi
	;;

	(C)
	# Pick the calendar file, overriding style and default.
	calendar=$optarg
	;;

	(n)
	# Show this many remaining events regardless of date.
	showcount=$optarg
	if (( showcount <= 0 )); then
	  print -r "$0: option -$opt requires a positive integer." >&2
	  return 1
	fi
	;;

	(S)
	# Explicitly specify a show programme, overriding style and default.
	# Colons in the argument are turned into space.
	showprog=(${(s.:.)optarg})
	;;
      esac
      ;;

      ###########################
      # Options without arguments
      ###########################
      (a)
      # Show all entries
      (( showall = 1 ))
      ;;

      (b)
      # Brief: don't show continuation lines
      (( brief = 1 ))
      ;;

      (d)
      # Move out of date items to the done file.
      (( done = 1 ))
      ;;

      (D)
      # Don't use done; needed with sched
      (( nodone = 1 ))
      ;;

      (r)
      # Show all remaining options in the calendar, i.e.
      # respect start time but ignore end time.
      # Any argument is treated as a start time.
      (( remaining = 1 ))
      ;;

      (s)
      # Use the "sched" builtin to scan at the appropriate time.
      sched=sched
      (( done = 1 ))
      ;;

      (v)
      # Verbose
      verbose=1
      ;;

      (<->)
      # Shorthand for -n <->
      showcount=$opt
      ;;

      (*)
      print "$0: unrecognised option: -$opt" >&2
      return 1
      ;;
    esac
    opt=$optrest[1]
    optrest=$optrest[2,-1]
  done
done
calopts=($argv[1,opti])
shift $(( opti ))

# Use of donefile requires explicit or implicit option request, plus
# no explicit -D.  It may already be empty because of the style.
(( done && !nodone )) || donefile=

if (( $# > 1 || ($# == 1 && remaining) )); then
  if [[ $1 = now ]]; then
    start=$EPOCHSECONDS
  elif [[ $1 = <-> ]]; then
    start=$1
  else
    if ! calendar_scandate -a $1; then
      print "$0: failed to parse date/time: $1" >&2
      return 1
    fi
    start=$REPLY
  fi
  shift
else
  # Get the time at which today started.
  y=${(%):-"%D{%Y}"} m=${(%):-"%D{%m}"} d=${(%):-"%D{%d}"}
  strftime -s today -r "%Y/%m/%d" "$y/$m/$d"
  start=$today
fi
# day of week of start time
strftime -s wd "%u" $start

if (( $# && !remaining )); then
  if [[ $1 = +* ]]; then
    if ! calendar_scandate -a -R $start ${1[2,-1]}; then
      print "$0: failed to parse relative time: $1" >&2
      return 1
    fi
    (( stop = REPLY ))
  elif [[ $1 = <-> ]]; then
    stop=$1
  else
    if ! calendar_scandate -a $1; then
      print "$0: failed to parse date/time: $1" >&2
      return 1
    fi
    stop=$REPLY
  fi
  if (( stop < start )); then
    strftime -s REPLY $datefmt $start
    strftime -s REPLY2 $datefmt $stop
    print "$0: requested end time is before start time:
  start: $REPLY
  end: $REPLY2" >&2
    return 1
  fi
  shift
else
  # By default, show 2 days.  If it's Friday (5) show up to end
  # of Monday (4) days; likewise on Saturday show 3 days.
  # If -r, this is calculated but not used.  This is paranoia,
  # to avoid an unusable value of stop; but it shouldn't get used.
  case $wd in
    (5)
    ndays=4
    ;;

    (6)
    ndays=3
    ;;

    (*)
    ndays=2
    ;;
  esac
  stop=$(( start + ndays * 24 * 60 * 60 ))
fi

if (( $# )); then
  print "Usage: $0 [ start-date-time stop-date-time ]" >&2
  return 1
fi

autoload -Uz matchdate

[[ -n $donefile ]] && rm -f $newfile

if (( verbose )); then
  print -n "start: "
  strftime $datefmt $start
  print -n "stop: "
  if (( remaining )); then
    print "none"
  else
    strftime $datefmt $stop
  fi
fi

local mycmds="${TMPPREFIX:-/tmp/zsh}.calendar_cmds.$$"
zf_ln -fn =(<<<'') $mycmds || return 1

# start of subshell for OS file locking
(
# start of block for following always to clear up lockfiles.
# Not needed but harmless if OS file locking is used.
{
  if [[ -n $donefile ]]; then
    # Attempt to lock both $donefile and $calendar.
    # Don't lock $newfile; we've tried our best to make
    # the name unique.
    if zmodload -F zsh/system b:zsystem && zsystem supports flock &&
      zsystem flock $calendar 2>/dev/null &&
      zsystem flock $donefile 2>/dev/null; then
      # locked OK
      :
    else
      calendar_lockfiles $calendar $donefile || exit 1
    fi
  fi

  calendar_read $calendar
  for line in $calendar_entries; do
    calendar_parse $line  ||  continue

    # Extract returned parameters from $reply
    # Time of event
    (( t = ${reply[time]} ))
    # Remainder of line including RPT and WARN stuff:  we need
    # to keep these for rescheduling.
    restline=$reply[text1]
    # Look for specific warn time.
    if [[ -n ${reply[warntime]} ]]; then
      (( mywarntime = t - ${reply[warntime]} ))
      mywarnstr=${reply[warnstr]}
    else
      (( mywarntime = warntime ))
      mywarnstr=$warnstr
    fi
    # Look for a repeat time.
    if [[ -n ${reply[rpttime]} ]]; then
      # The actual time of the next event, which appears as text
      (( repeattime = ${reply[rpttime]} ))
      (( repeating = 1 ))
    else
      (( repeating = 0 ))
    fi
    # Finished extracting parameters from $reply

    if (( verbose )); then
      print "Examining: $line"
      print -n "  Date/time: "
      strftime $datefmt $t
      if [[ -n $sched ]]; then
	print "  Warning $mywarntime seconds ($mywarnstr) before"
      fi
    fi
    (( shown = 0 ))
    if (( brief )); then
      tmplines=("${(f)line}")
      showline=${(F)${${tmplines[1,brief]}}}
    else
      showline=$line
    fi
    match=()
    # Strip continuation lines starting " #".
    while [[ $showline = (#b)(*$'\n')[[:space:]]##\#[^$'\n']##(|$'\n'(*)) ]]; do
      showline="$match[1]$match[3]"
    done
    # Strip trailing empty lines
    showline=${showline%%[[:space:]]#}
    if (( showall || (t >= start && (remaining || t <= stop || icount < showcount)) ))
    then
      print -r -- ${(qq)showprog} $start $stop ${(qq)showline} >>$mycmds
      (( icount++ ))
      # Doesn't count as "shown" unless the event has now passed.
      (( t <= EPOCHSECONDS )) && (( shown = 1 ))
    elif [[ -n $sched ]]; then
      (( tsched = t - mywarntime ))
      if (( tsched >= start && tsched <= stop)); then
	showline="due in ${mywarnstr}: $showline"
	print -r -- ${(qq)showprog} $start $stop ${(qq)showline} >>$mycmds
      elif (( tsched < start )); then
	# We haven't actually shown it, but it's in the past,
	# so treat it the same.  Should probably rename this variable.
	(( shown = 1 ))
      fi
    fi
    if [[ -n $sched ]]; then
      if (( shown && repeating )); then
	# Done and dusted, but a repeated event is due.
	strftime -s newdate $datefmt $repeattime
	if [[ $newdate != *[[:space:]] && $restline != [[:space:]]* ]]; then
	  newdate+=" "
	fi
	calendar_addlines+=("$newdate$restline")

	# We'll add this back in below, but we check in case the
	# repeated event is the next one due.  It's not
	# actually a disaster if there's an error and we fail
	# to add the time.  Always try to reschedule this event.
	(( tcalc = repeattime, resched = 1 ))
      else
	(( tcalc = t ))
      fi

      if (( tcalc - mywarntime > EPOCHSECONDS )); then
	# schedule for a warning
	(( tsched = tcalc - mywarntime, resched = 1 ))
      else
	# schedule for event itself
	(( tsched = tcalc ))
	# but don't schedule unless the event has not yet been shown.
	(( !shown )) && (( resched = 1 ))
      fi
      if (( resched && (next < 0 || tsched < next) )); then
	(( next = tsched ))
      fi
    fi
    if [[ -n $donefile ]]; then
      if (( shown )); then
	# Done and dusted.
	if ! print -r $line >>$donefile; then
	  if (( done != 3 )); then
	    (( done = 3 ))
	    print "Failed to append to $donefile" >&2
	  fi
	elif (( done != 3 )); then
	  (( done = 2 ))
	fi
      else
	# Still not over.
	if ! print -r $line >>$newfile; then
	  if (( done != 3 )); then
	    (( done = 3 ))
	    print "Failed to append to $newfile" >&2
	  fi
	elif (( done != 3 )); then
	  (( done = 2 ))
	fi
      fi
    fi
  done

  if [[ -n $sched ]]; then
    if [[ $next -ge 0 ]]; then
      # Remove any existing calendar scheduling.
      i=${"${(@)zsh_scheduled_events#*:*:}"[(I)calendar -s*]}
      {
        (( i )) && print sched -$i
        print -r -- ${(qq)sched} $next calendar "${calopts[@]}" $next $next
      } >>$mycmds
    else
      showline="No more calendar events: calendar not rescheduled.
Run \"calendar -s\" again if you add to it."
      print -r -- ${(qq)showprog} $start $stop ${(qq)showline} >>$mycmds
    fi
  fi

  if (( done == 2 )); then
    if ! mv $calendar $calendar.old; then
      print "Couldn't back up $calendar to $calendar.old.
New calendar left in $newfile." >&2
      (( rstat = 1 ))
    elif ! mv $newfile $calendar; then
      print "Failed to rename $newfile to $calendar.
Old calendar left in $calendar.old." >&2
      (( rstat = 1 ))
    fi
    nobackup=-B
  elif [[ -n $donefile ]]; then
    rm -f $newfile
  fi

  # Reschedule repeating events.
  for line in $calendar_addlines; do
    calendar_add -L $nobackup $line
  done
} always {
  (( ${#lockfiles} )) && rm -f $lockfiles
}

exit $rstat
) && {
  # Tasks that need to be in the current shell
  [[ -s $mycmds ]] && . $mycmds
  rm -f $mycmds
}
