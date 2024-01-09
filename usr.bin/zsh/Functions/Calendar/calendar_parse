# Parse the line passed down in the first argument as a calendar entry.
# Sets the values parsed into the associative array reply, consisting of:
# time  The time as an integer (as per EPOCHSECONDS) of the (next) event.
# text1 The text from the line not including the date/time, but
#       including any WARN or RPT text.  This is useful for rescheduling
#       events, since the keywords need to be retained in this case.
# warntime  Any warning time (WARN keyword) as an integer, else an empty
#       string.  This is the time of the warning in units of EPOCHSECONDS,
#       not the parsed version of the original number (which was a time
#       difference).
# warnstr  Any warning time as the original string (e.g. "5 mins"), not
#       including the WARN keyword.
# schedrpttime The next scheduled recurrence (which may be cancelled
#              or rescheduled).
# rpttime The actual occurrence time:  the event may have been rescheduled,
#         in which case this is the time of the actual event (for use in
#         programming warnings etc.) rather than that of the normal
#         recurrence (which is recorded by calendar_add as RECURRENCE).
#
# rptstr   Any repeat/recurrence time as the original string.
# text2    The text from the line with the date and other keywords and
#          values removed.
#
# Note that here an "integer" is a string of digits, not an internally
# formatted integer.
#
# Return status 1 if parsing failed.  reply is set to an empty
# in this case.  Note the caller is responsible for
# making reply local.

emulate -L zsh
setopt extendedglob

local vdatefmt="%Y%m%dT%H%M%S"

local REPLY REPLY2 timefmt occurrence skip try_to_recover before after
local -a match mbegin mend
integer now then replaced firstsched schedrpt
# Any text matching "OCCURRENCE <timestamp> <disposition>"
# may occur multiple times.  We set occurrences[<timestamp>]=disposition.
local -A occurrences

autoload -Uz calendar_scandate

typeset -gA reply

reply=()

if (( $# != 1 )); then
  print "Usage: $0 calendar-entry" >&2
  return 2
fi

# This call sets REPLY to the date and time in seconds since the epoch,
# REPLY2 to the line with the date and time removed.
calendar_scandate -as $1 || return 1
reply[time]=$(( REPLY ))
schedrpt=${reply[time]}
reply[text1]=${REPLY2##[[:space:]]#}
reply[text2]=${reply[text1]}

while true; do

  case ${reply[text2]} in
    # First check for a scheduled repeat time.  If we don't find one
    # we'll use the normal time.
    ((#b)(*[[:space:]\#])RECURRENCE[[:space:]]##([^[:space:]]##)([[:space:]]*|))
    strftime -rs then $vdatefmt ${match[2]} ||
    print "format: $vdatefmt, string ${match[2]}" >&2
    schedrpt=$then
    reply[text2]="${match[1]}${match[3]##[ 	]#}"
    ;;

    # Look for specific warn time.
    ((#b)(|*[[:space:],])WARN[[:space:]](*))
    if calendar_scandate -asm -R $reply[time] $match[2]; then
      reply[warntime]=$REPLY
      reply[warnstr]=${match[2]%%"$REPLY2"}
      # Remove spaces and tabs but not newlines from trailing text,
      # else the formatting looks funny.
      reply[text2]="${match[1]}${REPLY2##[ 	]#}"
    else
      # Just remove the keyword for further parsing
      reply[text2]="${match[1]}${match[2]##[ 	]#}"
    fi
    ;;

    ((#b)(|*[[:space:],])RPT[[:space:]](*))
    before=${match[1]}
    after=${match[2]}
    if [[ $after = CANCELLED(|[[:space:]]*) ]]; then
      reply[text2]="$before${match[2]##[ 	]#}"
      reply[rptstr]=CANCELLED
      reply[rpttime]=CANCELLED
      reply[schedrpttime]=CANCELLED
    elif calendar_scandate -a -R $schedrpt $after; then
      # It's possible to calculate a recurrence, however we don't
      # do that yet.  For now just keep the current time as
      # the recurrence.  Hence we ignore REPLY.
      reply[text2]="$before${REPLY2##[	]#}"
      reply[rptstr]=${after%%"$REPLY2"}
      # Until we find an individual occurrence, the actual time
      # of the event is the regular one.
      reply[rpttime]=$schedrpt
    else
      # Just remove the keyword for further parsing
      reply[text2]="$before${after##[[:space:]]#}"
    fi
    ;;

    ((#b)(|*[[:space:]\#])OCCURRENCE[[:space:]]##([^[:space:]]##)[[:space:]]##([^[:space:]]##)(*))
    occurrences[${match[2]}]="${match[3]}"
    # as above
    reply[text2]="${match[1]}${match[4]##[ 	]#}"
    ;;

    (*)
    break
    ;;
  esac
done

if [[ -n ${reply[rpttime]} && ${reply[rptstr]} != CANCELLED ]]; then
  # Recurring event.  We need to find out when it recurs.
  (( now = EPOCHSECONDS ))

  # First find the next recurrence.
  replaced=0
  reply[schedrpttime]=$schedrpt
  if (( schedrpt >= now )); then
    firstsched=$schedrpt
  fi
  while (( ${reply[schedrpttime]} < now || replaced )); do
    if ! calendar_scandate -a -R ${reply[schedrpttime]} ${reply[rptstr]}; then
      break
    fi
    if (( REPLY <= ${reply[schedrpttime]} )); then
      # going backwards --- pathological case
      break;
    fi
    reply[schedrpttime]=$REPLY
    reply[rpttime]=$REPLY
    if (( ${reply[schedrpttime]} > now && firstsched == 0 )); then
      firstsched=$REPLY
    fi
    replaced=0
    # do we have an occurrence to compare against?
    if (( ${#occurrences} )); then
      strftime -s timefmt $vdatefmt ${reply[schedrpttime]}
      occurrence=$occurrences[$timefmt]
      if [[ -n $occurrence ]]; then
	# Yes, this replaces the scheduled one.
	replaced=1
      fi
    fi
  done
  # Now look through occurrences (values only) and see which are (i) still
  # to happen (ii) early than the current rpttime.
  for occurrence in $occurrences; do
    if [[ $occurrence != CANCELLED ]]; then
      strftime -rs then $vdatefmt $occurrence ||
      print "format: $vdatefmt, string $occurrence" >&2
      if (( then > now && then < ${reply[rpttime]} )); then
	reply[rpttime]=$then
      fi
    fi
  done
  # Finally, update the scheduled repeat time to the earliest
  # possible value.  This is so that if an occurrence replacement is
  # cancelled we pick up the regular one.  Can this happen?  Dunno.
  reply[schedrpttime]=$firstsched
fi

reply[text2]="${reply[text2]##[[:space:],]#}"

return 0
