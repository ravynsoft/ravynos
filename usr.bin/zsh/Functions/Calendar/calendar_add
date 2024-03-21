#!/bin/zsh
# All arguments are joined with spaces and inserted into the calendar
# file at the appropriate point.
#
# While the function compares the date of the new entry with dates in the
# existing calendar file, it does not do any sorting; it inserts the new
# entry before the first existing entry with a later date and time.

emulate -L zsh
setopt extendedglob # xtrace

local context=":datetime:calendar_add:"
local vdatefmt="%Y%m%dT%H%M%S"
local d='[[:digit:]]'

local calendar newfile REPLY lastline opt text occur
local -a calendar_entries lockfiles reply occurrences
integer my_date done rstat nolock nobackup new_recurring
integer keep_my_uid
local -A reply parse_new parse_old
local -a match mbegin mend
local my_uid their_uid

autoload -Uz calendar_{parse,read,lockfiles}

while getopts "BL" opt; do
  case $opt in
    (B)
    nobackup=1
    ;;

    (L)
    nolock=1
    ;;

    (*)
    return 1
    ;;
  esac
done
shift $(( OPTIND - 1 ))

# Read the calendar file from the calendar-file style
zstyle -s $context calendar-file calendar ||
  calendar=~/calendar
newfile=$calendar.new.$HOST.$$

local addline="$*"
if ! calendar_parse $addline; then
  print "$0: failed to parse date/time" >&2
  return 1
fi
parse_new=("${(@kv)reply}")
(( my_date = $parse_new[time] ))
if zstyle -t $context reformat-date; then
  local datefmt
  zstyle -s $context date-format datefmt ||
    datefmt="%a %b %d %H:%M:%S %Z %Y"
  strftime -s REPLY $datefmt $parse_new[time]
  addline="$REPLY $parse_new[text1]"
fi
if [[ -n $parse_new[rptstr] ]]; then
  (( new_recurring = 1 ))
  if [[ $parse_new[rptstr] = CANCELLED ]]; then
    (( done = 1 ))
  elif [[ $addline = (#b)(*[[:space:]\#]RECURRENCE[[:space:]]##)([^[:space:]]##)([[:space:]]*|) ]]; then
    # Use the updated recurrence time
    strftime -s REPLY $vdatefmt ${parse_new[schedrpttime]}
    addline="${match[1]}$REPLY${match[3]}"
  else
    # Add a recurrence time
    [[ $addline = *$'\n' ]] || addline+=$'\n'
    strftime -s REPLY $vdatefmt ${parse_new[schedrpttime]}
    addline+="  # RECURRENCE $REPLY"
  fi
fi

# $calendar doesn't necessarily exist yet.

# Match a UID, a unique identifier for the entry inherited from
# text/calendar format.
local uidpat='(|*[[:space:]])UID[[:space:]]##(#b)([[:xdigit:]]##)(|[[:space:]]*)'
if [[ $addline = ${~uidpat} ]]; then
  my_uid=${(U)match[1]}
fi

# start of subshell for OS file locking
(
# start of block for following always to clear up lockfiles.
# Not needed but harmless if OS file locking is used.
{
  if (( ! nolock )); then
    if zmodload -F zsh/system b:zsystem && zsystem supports flock &&
      zsystem flock $calendar 2>/dev/null; then
      # locked OK
      :
    else
      calendar_lockfiles $calendar || exit 1
    fi
  fi

  if [[ -f $calendar ]]; then
    calendar_read $calendar

    if [[ -n $my_uid ]]; then
      # Pre-scan to events with the same UID
      for line in $calendar_entries; do
	calendar_parse $line  ||  continue
	parse_old=("${(@kv)reply}")
	# Recurring with a UID?
	if [[ $line = ${~uidpat} ]]; then
	  their_uid=${(U)match[1]}
	  if [[ $their_uid = $my_uid ]]; then
	    # Deal with recurrences and also some add some
	    # extra magic for cancellation.

	    # See if we have found a recurrent version
	    if [[ -z $parse_old[rpttime] ]]; then
	      # No, so assume this is a straightforward replacement
	      # of a non-recurring event.

	      # Is this a cancellation of a non-recurring event?
	      # Look for an OCCURRENCE in the form
	      #   OCCURRENCE 20100513T110000 CANCELLED
	      # although we don't bother looking at the date/time---
	      # it's one-off, so this should already be unique.
	      if [[ $new_recurring -eq 0 && \
		$parse_new[text1] = (|*[[:space:]\#])"OCCURRENCE"[[:space:]]##([^[:space:]]##[[:space:]]##CANCELLED)(|[[:space:]]*) ]]; then
		# Yes, so skip both the old and new events.
		(( done = 1 ))
	      fi
	      # We'll skip this UID when we encounter it again.
	      continue
	    fi
	    if (( new_recurring )); then
	      # Replacing a recurrence; there can be only one.
	      # TBD: do we replace existing occurrences of the
	      # replaced recurrent event?  I'm guessing not, but
	      # if we keep the UID then maybe we should.
	      #
	      # TBD: ick, suppose we're cancelling an even that
	      # we added to a recurring sequence but didn't replace
	      # the recurrence.  We might get RPT CANCELLED for this.
	      # That would be bad.  Should invent better way of
	      # cancelling non-recurring event.
	      continue
	    else
	      # The recorded event is recurring, but the new one is a
	      # one-off event. If there are embedded OCCURRENCE declarations,
	      # use those.
	      #
	      # TBD: We could be clever about text associated
	      # with the occurrence.  Copying the entire text
	      # of the meeting seems like overkill but people often
	      # add specific remarks about why this occurrence was
	      # moved/cancelled.
	      #
	      # TBD: one case we don't yet handle is cancelling
	      # something that isn't replacing a recurrence, i.e.
	      # something we added as OCCURRENCE XXXXXXXXTXXXXXX <when>.
	      # If we're adding a CANCELLED occurrence we should
	      # look to see if it matches <when> and if so simply
	      # delete that occurrence.
	      #
	      # TBD: one nasty case is if the new occurrence
	      # occurs before the current scheduled time.  As we
	      # never look backwards we'll miss it.
	      text=$addline
	      occurrences=()
	      while [[ $text = (#b)(|*[[:space:]\#])"OCCURRENCE"[[:space:]]##([^[:space:]]##[[:space:]]##[^[:space:]]##)(|[[:space:]]*) ]]; do
		occurrences+=($match[2])
		text="$match[1] $match[3]"
	      done
	      if (( ! ${#occurrences} )); then
		# No embedded occurrences.  We'll manufacture one
		# that doesn't refer to an original recurrence.
		strftime -s REPLY $vdatefmt $my_date
		occurrences=("XXXXXXXXTXXXXXX $REPLY")
	      fi
	      # Add these occurrences, checking if they replace
	      # an existing one.
	      for occur in ${(o)occurrences}; do
		REPLY=${occur%%[[:space:]]*}
		# Only update occurrences that refer to genuine
		# recurrences.
		if [[ $REPLY = [[:digit:]](#c8)T[[:digit:]](#c6) && $line = (#b)(|*[[:space:]\#])(OCCURRENCE[[:space:]]##)${REPLY}[[:space:]]##[^[:space:]]##(|[[:space:]]*) ]]; then
		  # Yes, update in situ
		  line="${match[1]}${match[2]}$occur${match[3]}"
		else
		  # No, append.
		  [[ $line = *$'\n' ]] || line+=$'\n'
		  line+="  # OCCURRENCE $occur"
		fi
	      done
	      # The line we're adding now corresponds to the
	      # original event.  We'll skip the matching UID
	      # in the file below, however.
	      addline=$line
	      # We need to work out which event is next, so
	      # reparse.
	      if calendar_parse $addline; then
		parse_new=("${(@kv)reply}")
		(( my_date = ${parse_new[time]} ))
		if zstyle -t $context reformat-date; then
		  zstyle -s $context date-format datefmt
		  strftime -s REPLY $datefmt $parse_new[time]
		  addline="$REPLY $parse_new[text1]"
		fi
	      fi
	    fi
	  fi
	fi
      done
    fi

    {
      for line in $calendar_entries; do
	calendar_parse $line  ||  continue
	parse_old=("${(@kv)reply}")
	if (( ! done && ${parse_old[time]} > my_date )); then
	  print -r -- $addline
	  (( done = 1 ))
	fi
	# We've already merged any information on the same UID
	# with our new text, probably.
	if [[ $keep_my_uid -eq 0 && -n $my_uid && $line = ${~uidpat} ]]; then
	  their_uid=${(U)match[1]}
	  [[ $my_uid = $their_uid ]] && continue
	fi
	if [[ $parse_old[time] -eq $my_date && $line = $addline ]]; then
	  (( done )) && continue # paranoia: shouldn't happen
	  (( done = 1 ))
	fi
	print -r -- $line
      done
      (( done )) || print -r -- $addline
    } >$newfile
    if (( ! nobackup )); then
      if ! mv $calendar $calendar.old; then
	print "Couldn't back up $calendar to $calendar.old.
New calendar left in $newfile." >&2
	(( rstat = 1 ))
      fi
    fi
  else
    (( done )) || print -r -- $addline >$newfile
  fi

  if (( !rstat )) && ! mv $newfile $calendar; then
    print "Failed to rename $newfile to $calendar.
Old calendar left in $calendar.old." >&2
    (( rstat = 1 ))
  fi
} always {
  (( ${#lockfiles} )) && rm -f $lockfiles
}

exit $rstat
)
