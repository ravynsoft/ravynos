# Scan a line for various common date and time formats.
# Set REPLY to the number of seconds since the epoch at which that
# time occurs.  The time does not need to be matched; this will
# produce midnight at the start of the date.
#
# Absolute times
#
# The rules below are fairly complicated, to allow any natural (and
# some highly unnatural but nonetheless common) combination of
# time and date used by English speakers.  It is recommended that,
# rather than exploring the intricacies of the system, users find
# a date format that is natural to them and stick to it.  This
# will avoid unexpected effects.  Various key facts should be noted,
# explained in more detail below:
#
# - In particular, note the confusion between month/day/year and
#   day/month/year when the month is numeric; this format should be
#   avoided if at all possible.  Many alternatives are available.
# - However, there is currently no localization support, so month
#   names must be English (though only the first three letters are required).
#   The same applies to days of the week if they occur (they are not useful).
# - The year must be given in full to avoid confusion, and only years
#   from 1900 to 2099 inclusive are matched.
# - Although timezones are parsed (complicated formats may not be recognized),
#   they are then ignored; no time adjustment is made.
# - Embedding of times within dates (e.g. "Wed Jun 16 09:30:00 BST 2010")
#   causes horrific problems because of the combination of the many
#   possible date and time formats to match.  The approach taken
#   here is to match the time, remove it, and see if the nearby text
#   looks like a date.  The problem is that the time matched may not
#   be that associated with the date, in which case the time will be
#   ignored.  To minimise this, when the argument "-a" is given to
#   anchor the date/time to the start of the line, we never look
#   beyond a newline.  So if any date/time strings in the text
#   are on separate lines the problem is avoided.
# - If you feel sophisticated enough and wish to avoid any ambiguity,
#   you can use RFC 2445 date/time strings, for example 20100601T170000.
#   These are parsed in one go.
#
# The following give some obvious examples; users finding here
# a format they like and not subject to vagaries of style may skip
# the full description.  As dates and times are matched separately
# (even though the time may be embedded in the date), any date format
# may be mixed with any format for the time of day provide the
# separators are clear (whitespace, colons, commas).
#   2007/04/03 13:13
#   2007/04/03:13:13
#   2007/04/03 1:13 pm
#   3rd April 2007, 13:13
#   April 3rd 2007 1:13 p.m.
#   Apr 3, 2007 13:13
#   Tue Apr 03 13:13:00 2007
#   13:13 2007/apr/3
#
# Times are parsed and extracted before dates.  They must use colons
# to separate hours and minutes, though a dot is allowed before seconds
# if they are present.  This limits time formats to
#   HH:MM[:SS[.FFFFF]] [am|pm|a.m.|p.m.]
#   HH:MM.SS[.FFFFF] [am|pm|a.m.|p.m.]
# in which square brackets indicate optional elements, possibly with
# alternatives.  Fractions of a second are recognised but ignored.
# Unless -r or -R are given (see below), a date is mandatory but a time of day is
# not; the time returned is at the start of the date.
#
# Time zones are not handled, though if one is matched following a time
# specification it will be removed to allow a surrounding date to be
# parsed.  This only happens if the format of the timezone is not too
# wacky:
#   +0100
#   GMT
#   GMT-7
#   CET+1CDT
# etc. are all understood, but any part of the timezone that is not numeric
# must have exactly three capital letters in the name.
#
# Dates suffer from the ambiguity between DD/MM/YYYY and MM/DD/YYYY.  It is
# recommended this form is avoided with purely numeric dates, but use of
# ordinals, eg. 3rd/04/2007, will resolve the ambiguity as the ordinal is
# always parsed as the day of the month.  Years must be four digits (and
# the first two must be 19 or 20); 03/04/08 is not recognised.  Other
# numbers may have leading zeroes, but they are not required.  The
# following are handled:
#   YYYY/MM/DD
#   YYYY-MM-DD
#   YYYY/MNM/DD
#   YYYY-MNM-DD
#   DD[th|st|rd] MNM[,] YYYY
#   DD[th|st|rd] MNM[,]            current year assumed
#   MNM DD[th|st|rd][,] YYYY
#   MNM DD[th|st|rd][,]            current year assumed
#   DD[th|st|rd]/MM[,] YYYY
#   DD[th|st|rd]/MM/YYYY
#   MM/DD[th|st|rd][,] YYYY
#   MM/DD[th|st|rd]/YYYY
# Here, MNM is at least the first three letters of a month name,
# matched case-insensitively.  The remainder of the month name may appear but
# its contents are irrelevant, so janissary, febrile, martial, apricot,
# etc. are happily handled.
#
# Note there are only two cases that assume the current year, the
# form "Jun 20" or "14 September" (the only two commonly occurring
# forms, apart from a "the" in some forms of English, which isn't
# currently supported).  Such dates will of course become ambiguous
# in the future, so should ideally be avoided.
#
# Times may follow dates with a colon, e.g. 1965/07/12:09:45; this
# is in order to provide a format with no whitespace.  A comma
# and whitespace are allowed, e.g. "1965/07/12, 09:45".
# Currently the order of these separators is not checked, so
# illogical formats such as "1965/07/12, : ,09:45" will also
# be matched.  Otherwise, a time is only recognised as being associated
# with a date if there is only whitespace in between, or if the time
# was embedded in the date.
#
# Days of the week are not scanned, but will be ignored if they occur
# at the start of the date pattern only.
#
# For example, the standard date format:
#   Fri Aug 18 17:00:48 BST 2006
# is handled by matching HH:MM:SS and removing it together with the
# matched (but unused) time zone.  This leaves the following:
#   Fri Aug 18 2006
# "Fri" is ignored and the rest is matched according to the sixth of
# the standard rules.
#
# Relative times
# ==============
#
# The option -r allows a relative time.  Years (or ys, yrs, or without s),
# months (or mths, mons, mnths, months, or without s --- "m", "ms" and
# "mns" are ambiguous and are not handled), weeks (or ws, wks, or without
# s) and days (or ds, dys, days, or without s), hours (or hs, hrs, with or
# without s), minutes (or mins, with or without s) and seconds (or ss,
# secs, with or without s) are understood.  Spaces between the numbers
# are optional, but are required between items, although a comma
# may be used (with or without spaces).
#
# Note that a year here is 365.25 days and a month is 30 days.
#
# With -R start_time, a relative time is parsed and start_time is treated
# as the start of the period.  This allows months and years to be calculated
# accurately.  If the option -m (minus) is also given the relative time is
# taken backwards from the start time.
#
# This allows forms like:
#   30 years 3 months 4 days 3:42:41
#   14 days 5 hours
#   4d,10hr
# In this case absolute dates are ignored.

emulate -L zsh
setopt extendedglob # xtrace

zmodload -i zsh/datetime || return 1

# separator characters before time or between time and date
# allow , - or : before the time: this allows spaceless but still
# relatively logical dates like 2006/09/19:14:27
# don't allow / before time !  the above
# is not 19 hours 14 mins and 27 seconds after anything.
local tschars="[-,:[:blank:]]"
# start pattern for time when anchored
local tspat_anchor="(${tschars}#)"
# ... when not anchored
local tspat_noanchor="(|*${tschars})"
# separator characters between elements.  comma is fairly
# natural punctuation; otherwise only allow whitespace.
local schars="[.,[:space:]]"
local -a dayarr
dayarr=(sun mon tue wed thu fri sat)
local daypat="${schars}#((#B)(${(j.|.)dayarr})[a-z]#~month*)"
# Start pattern for date: treat , as space for simplicity.  This
# is illogical at the start but saves lots of minor fiddling later.
# Date start pattern when anchored at the start.
# We need to be able to ignore the day here, although (for consistency
# with the unanchored case) we don't remove it until later.
# (The problem in the other case is that matching anything before
# the day of the week is greedy, so the day of the week gets ignored
# if it's optional.)
local dspat_anchor="(|(#B)(${daypat}|)(#b)${schars}#)"
local dspat_anchor_noday="(|${schars}#)"
# Date start pattern when not anchored at the start.
local dspat_noanchor="(|*${schars})"
# end pattern for relative times: similar remark about use of $schars.
local repat="(|s)(|${schars}*)"
# not locale-dependent!  I don't know how to get the months out
# of the system for the purpose of finding out where they occur.
# We may need some completely different heuristic.
local monthpat="(jan|feb|mar|apr|may|jun|jul|aug|sep|oct|nov|dec)[a-z]#"
integer daysecs=$(( 24 * 60 * 60 ))
local d="[[:digit:]]"

integer year year2 month month2 day day2 hour minute second then nth wday wday2
local opt line orig_line mname MATCH MBEGIN MEND tz test rest_line
local -a match mbegin mend
# Flags that we found a date or a time (maybe a relative time)
integer date_found time_found
# Flag that it's OK to have a time only
integer time_ok
# Indices of positions of start and end of time and dates found.
# These are actual character indices as zsh would normally use, i.e.
# line[time_start,time_end] is the string for the time.
integer time_start time_end date_start date_end
integer anchor anchor_end debug setvar
integer relative relative_start reladd reldate relsign=1 newadd h1 h2 hd

while getopts "aAdmrR:st" opt; do
  case $opt in
    (a)
    # anchor
    (( anchor = 1 ))
    ;;

    (A)
    # anchor at end, too
    (( anchor = 1, anchor_end = 1 ))
    ;;

    (d)
    # enable debug output
    (( debug = 1 ))
    ;;

    (m)
    # relative with negative offsets
    (( relsign = -1 ))
    ;;

    (r)
    # relative with no fixed start
    (( relative = 1 ))
    ;;

    (R)
    # relative with fixed start supplied
    (( relative_start = OPTARG, relative = 2 ))
    ;;

    (s)
    (( setvar = 1 ))
    ;;

    (t)
    (( time_ok = 1 ))
    ;;

    (*)
    return 1
    ;;
  esac
done
shift $(( OPTIND - 1 ))

line=$1

local dspat dspat_noday tspat
if (( anchor )); then
  # Anchored at the start.
  dspat=$dspat_anchor
  dspat_noday=$dspat_anchor_noday
  if (( relative )); then
    tspat=$tspat_anchor
  else
    # We'll test later if the time is associated with the date.
    tspat=$tspat_noanchor
  fi
  # We can save a huge amount of grief (I've discovered) if when
  # we're anchored to the start we ignore anything after a newline.
  # However, don't do this if we're anchored to the end.  The
  # match should fail if there are extra lines in that case.
  if [[ anchor_end -eq 0 && $line = (#b)([^$'\n']##)($'\n'*) ]]; then
    line=$match[1]
    rest_line=$match[2]
  fi
else
  dspat=$dspat_noanchor
  dspat_noday=$dspat_noanchor
  tspat=$tspat_noanchor
fi
orig_line=$line

# Look for a time separately; we need colons for this.
# We want to look for the first time to ensure it's associated
# with a date at the start of the line.  Of course there may be
# a time followed by some other text followed by a date, but
# in that case the whole thing is too ambiguous to worry about
# (and we don't need to worry about this for a calendar entry where
# the date must be at the start).
#
# We do this by minimal matching at the head, i.e. ${...#...}.
# To use a case statement we'd need to be able to request non-greedy
# matching for a pattern.
local rest
# HH:MM:SECONDS am/pm with optional decimal seconds
rest=${line#(#ibm)${~tspat}(<0-12>):(<0-59>)[.:]((<0-59>)(.<->|))[[:space:]]#([ap])(|.)[[:space:]]#m(.|[[:space:]]|(#e))}
if [[ $rest != $line ]]; then
  hour=$match[2]
  minute=$match[3]
  second=$match[5]
  [[ $match[7] = (#i)p ]] && (( hour <= 12 )) && (( hour += 12 ))
  time_found=1
fi
if (( time_found == 0 )); then
  # no seconds, am/pm
  rest=${line#(#ibm)${~tspat}(<0-12>):(<0-59>)[[:space:]]#([ap])(|.)[[:space:]]#m(.|[[:space:]]|(#e))}
  if [[ $rest != $line ]]; then
    hour=$match[2]
    minute=$match[3]
    [[ $match[4] = (#i)p ]] && (( hour <= 12 )) && (( hour += 12 ))
    time_found=1
  fi
fi
if (( time_found == 0 )); then
  # no colon, even, but a.m./p.m. indicator
  rest=${line#(#ibm)${~tspat}(<0-12>)[[:space:]]#([ap])(|.)[[:space:]]#m(.|[[:space:]]|(#e))}
  if [[ $rest != $line ]]; then
    hour=$match[2]
    minute=0
    [[ $match[3] = (#i)p ]] && (( hour <= 12 )) && (( hour += 12 ))
    time_found=1
  fi
fi
if (( time_found == 0 )); then
  # 24 hour clock, with seconds
  rest=${line#(#ibm)${~tspat}(<0-24>):(<0-59>)[.:]((<0-59>)(.<->|))(.|[[:space:]]|(#e))}
  if [[ $rest != $line ]]; then
    hour=$match[2]
    minute=$match[3]
    second=$match[5]
    time_found=1
  fi
fi
if (( time_found == 0 )); then
  rest=${line#(#ibm)${~tspat}(<0-24>):(<0-59>)(.|[[:space:]]|(#e))}
  if [[ $rest != $line ]]; then
    hour=$match[2]
    minute=$match[3]
    time_found=1
  fi
fi
if (( time_found == 0 )); then
  # Combined date and time formats:  here we can use an anchor because
  # we know the complete format.
  (( anchor )) && tspat=$tspat_anchor
  # RFC 2445
  rest=${line#(#ibm)${~tspat}(|\"[^\"]##\":)($~d$~d$~d$~d)($~d$~d)($~d$~d)T($~d$~d)($~d$~d)($~d$~d)([[:space:]]#|(#e))}
  if [[ $rest != $line ]]; then
    year=$match[3]
    month=$match[4]
    day=$match[5]
    hour=$match[6]
    minute=$match[7]
    second=$match[8]
    # signal don't need to take account of time in date...
    time_found=2
    date_found=1
    date_start=$mbegin[3]
    date_end=$mend[-1]
  fi
fi
(( hour == 24 )) && hour=0

if (( time_found && ! date_found )); then
  # time was found; if data also found already, process below.
  time_start=$mbegin[2]
  time_end=$mend[-1]
  # Remove the timespec because it may be in the middle of
  # the date (as in the output of "date".
  # There may be a time zone, too, which we don't yet handle.
  # (It's not in POSIX strptime() and libraries don't support it well.)
  # This attempts to remove some of the weirder forms.
  if [[ $line[$time_end+1,-1] = (#b)[[:space:]]#([A-Z][A-Z][A-Z]|[-+][0-9][0-9][0-9][0-9])([[:space:]]|(#e))* || \
        $line[$time_end+1,-1] = (#b)[[:space:]]#([A-Z][A-Z][A-Z](|[-+])<0-12>)([[:space:]]|(#e))*  || \
        $line[$time_end+1,-1] = (#b)[[:space:]]#([A-Z][A-Z][A-Z](|[-+])<0-12>[A-Z][A-Z][A-Z])([[:space:]]|(#e))* ]]; then
     (( time_end += ${mend[-1]} ))
     tz=$match[1]
  fi
  line=$line[1,time_start-1]$line[time_end+1,-1]
  (( debug )) && print "line after time: $line"
fi

if (( relative == 0 && date_found == 0 )); then
  # Date.
  case $line in
  # Look for YEAR[-/.]MONTH[-/.]DAY
  ((#bi)${~dspat}((19|20)[0-9][0-9])[-/](<1-12>)[-/](<1-31>)*)
  year=$match[2]
  month=$match[4]
  day=$match[5]
  date_start=$mbegin[2] date_end=$mend[5]
  date_found=1
  ;;

  # Same with month name
  ((#bi)${~dspat}((19|20)[0-9][0-9])[-/]${~monthpat}[-/](<1-31>)*)
  year=$match[2]
  mname=$match[4]
  day=$match[5]
  date_start=$mbegin[2] date_end=$mend[5]
  date_found=1
  ;;

  # Look for DAY[th/st/nd/rd] MNAME[,] YEAR
  ((#bi)${~dspat}(<1-31>)(|th|st|nd|rd)[[:space:]]##${~monthpat}(|,)[[:space:]]##((19|20)[0-9][0-9])*)
  day=$match[2]
  mname=$match[4]
  year=$match[6]
  date_start=$mbegin[2] date_end=$mend[6]
  date_found=1
  ;;

  # Look for MNAME DAY[th/st/nd/rd][,] YEAR
  ((#bi)${~dspat}${~monthpat}[[:space:]]##(<1-31>)(|th|st|nd|rd)(|,)[[:space:]]##((19|20)[0-9][0-9])*)
  mname=$match[2]
  day=$match[3]
  year=$match[6]
  date_start=$mbegin[2] date_end=$mend[6]
  date_found=1
  ;;

  # Look for DAY[th/st/nd/rd] MNAME; assume current year
  ((#bi)${~dspat}(<1-31>)(|th|st|nd|rd)[[:space:]]##${~monthpat}(|,)([[:space:]]##*|))
  day=$match[2]
  mname=$match[4]
  strftime -s year "%Y" $EPOCHSECONDS
  date_start=$mbegin[2] date_end=$mend[5]
  date_found=1
  ;;

  # Look for MNAME DAY[th/st/nd/rd]; assume current year
  ((#bi)${~dspat}${~monthpat}[[:space:]]##(<1-31>)(|th|st|nd|rd)(|,)([[:space:]]##*|))
  mname=$match[2]
  day=$match[3]
  strftime -s year "%Y" $EPOCHSECONDS
  date_start=$mbegin[2] date_end=$mend[5]
  date_found=1
  ;;

  # Now it gets a bit ambiguous.
  # Look for DAY[th/st/nd/rd][/]MONTH[/ ,]YEAR
  ((#bi)${~dspat}(<1-31>)(|th|st|nd|rd)/(<1-12>)((|,)[[:space:]]##|/)((19|20)[0-9][0-9])*)
  day=$match[2]
  month=$match[4]
  year=$match[7]
  date_start=$mbegin[2] date_end=$mend[7]
  date_found=1
  ;;

  # Look for MONTH[/]DAY[th/st/nd/rd][/ ,]YEAR
  ((#bi)${~dspat}(<1-12>)/(<1-31>)(|th|st|nd|rd)((|,)[[:space:]]##|/)((19|20)[0-9][0-9])*)
  month=$match[2]
  day=$match[3]
  year=$match[7]
  date_start=$mbegin[2] date_end=$mend[7]
  date_found=1
  ;;

  # Look for WEEKDAY
  ((#bi)${~dspat_noday}(${~daypat})(|${~schars})*)
  integer wday_now wday
  local wdaystr=${(L)match[3]}
  date_start=$mbegin[2] date_end=$mend[2]

  # Find the day number.
  local -a wdays
  # This is the ordering of %w in strtfime (zero-offset).
  wdays=(sun mon tue wed thu fri sat sun)
  (( wday = ${wdays[(i)$wdaystr]} - 1 ))

  # Find the date for that day.
  (( then = EPOCHSECONDS ))
  strftime -s wday_now "%w" $then
  # Day is either today or in the past.
  (( wday_now < wday )) && (( wday_now += 7 ))
  (( then -= (wday_now - wday) * 24 * 60 * 60 ))
  strftime -s year "%Y" $then
  strftime -s month "%m" $then
  strftime -s day "%d" $then
  date_found=1
  ;;

  # Look for "today", "yesterday", "tomorrow"
  ((#bi)${~dspat_noday}(yesterday|today|tomorrow|now)(|${~schars})*)
  (( then = EPOCHSECONDS ))
  case ${(L)match[2]} in
    (yesterday)
    (( then -= daysecs ))
    ;;

    (tomorrow)
    (( then += daysecs ))
    ;;

    (now)
    time_found=1 time_end=0 time_start=1
    strftime -s hour "%H" $then
    strftime -s minute "%M" $then
    strftime -s second "%S" $then
    ;;
  esac
  strftime -s year "%Y" $then
  strftime -s month "%m" $then
  strftime -s day "%d" $then
  date_start=$mbegin[2] date_end=$mend[2]
  date_found=1
  ;;
  esac
fi

if (( date_found || (time_ok && time_found) )); then
  # date found
  # see if there's a day at the start
  if (( date_found )); then
    if [[ ${line[1,$date_start-1]} = (#bi)${~daypat}${~schars}# ]]; then
	    date_start=$mbegin[1]
    fi
    line=${line[1,$date_start-1]}${line[$date_end+1,-1]}
  fi
  if (( time_found == 1 )); then
    if (( date_found )); then
      # If we found a time, it must be associated with the date,
      # or we can't use it.  Since we removed the time from the
      # string to find the date, however, it's complicated to
      # know where both were found.  Reconstruct the date indices of
      # the original string.
      if (( time_start <= date_start )); then
	# Time came before start of date; add length in.
	(( date_start += time_end - time_start + 1 ))
      fi
      if (( time_start <= date_end )); then
	(( date_end += time_end - time_start + 1 ))
      fi

      if (( time_end + 1 < date_start )); then
	# If time wholly before date, OK if only separator characters
	# in between.  (This allows some illogical stuff with commas
	# but that's probably not important.)
	if [[ ${orig_line[time_end+1,date_start-1]} != ${~schars}# ]]; then
	  # Clearly this can't work if anchor is set.  In principle,
	  # we could match the date and ignore the time if it wasn't.
	  # However, that seems dodgy.
	  return 1
	else
	  # Form massaged line by removing the entire date/time chunk.
	  line="${orig_line[1,time_start-1]}${orig_line[date_end+1,-1]}"
	fi
      elif (( date_end + 1 < time_start )); then
	# If date wholly before time, OK if only time separator characters
	# in between.  This allows 2006/10/12:13:43 etc.
	if [[ ${orig_line[date_end+1,time_start-1]} != ${~tschars}# ]]; then
	  # Here, we assume the time is associated with something later
	  # in the line.  This is pretty much inevitable for the sort
	  # of use we are expecting.  For example,
	  #   2006/10/24  Meeting from early, may go on till 12:00.
	  # or with some uses of the calendar system,
	  #   2006/10/24 MR 1 Another pointless meeting WARN 01:00
	  # The 01:00 says warn an hour before, not that the meeting starts
	  # at 1 am.  About the only safe way round would be to force
	  # a time to be present, but that's not how the traditional
	  # calendar programme works.
	  #
	  # Hence we need to reconstruct.
	  (( time_found = 0, hour = 0, minute = 0, second = 0 ))
	  line="${orig_line[1,date_start-1]}${orig_line[date_end+1,-1]}"
	else
	  # As above.
	  line="${orig_line[1,date_start-1]}${orig_line[time_end+1,-1]}"
	fi
      fi
    else
      # Time only.
      # We didn't test anchors for time originally, since it
      # might have been embedded in the date.  If there's no date,
      # we need to test specially.
      if (( anchor )) &&
	[[ ${orig_line[1,time_start-1]} != ${~tschars}# ]]; then
	# Anchor at start failed.
	return 1
      fi
      strftime -s year "%Y" $EPOCHSECONDS
      strftime -s month "%m" $EPOCHSECONDS
      strftime -s day "%d" $EPOCHSECONDS
      # Date now handled.
      (( date_found = 1 ))
    fi
    if (( debug )); then
      print "Time string: $time_start,$time_end:" \
	"'$orig_line[time_start,time_end]'"
      (( date_ok )) && print "Date string: $date_start,$date_end:" \
	"'$orig_line[date_start,date_end]'"
      print "Remaining line: '$line$rest_line'"
    fi
  fi
fi

if (( relative )); then
  if (( relative == 2 )); then
    # Relative years and months are variable, and we may need to
    # be careful about days.
    strftime -s year "%Y" $relative_start
    strftime -s month "%m" $relative_start
    strftime -s day "%d" $relative_start
    strftime -rs then "%Y:%m:%d" "${year}:${month}:${day}"
  fi
  if [[ $line = (#bi)${~dspat}(<->|)[[:space:]]#(y|yr|year|yearly)${~repat} ]]; then
    [[ -z $match[2] ]] && match[2]=1
    if (( relative == 2 )); then
      # We need the difference between relative_start & the
      # time ${match[2]} years later.  This means keeping the month and
      # day the same and changing the year.
      (( year2 = year + relsign * ${match[2]} ))
      strftime -rs reldate "%Y:%m:%d" "${year2}:${month}:${day}"

      # If we've gone from a leap year to a non-leap year, go back a day.
      strftime -s month2 "%m" $reldate
      (( month2 != month )) && (( reldate -= daysecs ))

      # Keep this as a difference for now since we may need to add in other stuff.
      (( reladd += reldate - then ))
    else
      (( reladd += relsign * ((365*4+1) * daysecs * ${match[2]} + 1) / 4 ))
    fi
    line=${line[1,$mbegin[2]-1]}${line[$mend[4]+1,-1]}
    time_found=1
  fi
  if [[ $line = (#bi)${~dspat}(<->|)[[:space:]]#(mth|mon|mnth|month|monthly)${~repat} ]]; then
     [[ -z $match[2] ]] && match[2]=1
     if (( relative == 2 )); then
       # Need to add on ${match[2]} months as above.
       (( month2 = month + relsign * ${match[2]} ))
       if (( month2 <= 0 )); then
	 # going backwards beyond start of given year
	 (( year2 = year + month2 / 12 - 1, month2 = month2 + (year-year2)*12 ))
       else
	 (( year2 = year + (month2 - 1)/ 12, month2 = (month2 - 1) % 12 + 1 ))
       fi
       strftime -rs reldate "%Y:%m:%d" "${year2}:${month2}:${day}"

       # If we've gone past the end of the month because it was too short,
       # we have two options (i) get the damn calendar fixed (ii) wind
       # back to the end of the previous month.  (ii) is easier for now.
       if (( day > 28 )); then
	 while true; do
	   strftime -s day2 "%d" $reldate
	   # There are only up to 3 days in it, so just wind back one at a
           # time.  Saves counting.
	   (( day2 >= 28 )) && break
	   (( reldate -= daysecs ))
	 done
       fi

       (( reladd += reldate - then ))
     else
       (( reladd += relsign * 30 * daysecs * ${match[2]} ))
     fi
     line=${line[1,$mbegin[2]-1]}${line[$mend[4]+1,-1]}
     time_found=1
  fi
  # For the next three items we accumulate adjustments in "newadd".
  # See note below for why they are special.
  if [[ $relative = 2 && $line = (#bi)${~dspat_noday}(<->)(th|rd|nd|st)(${~daypat})(|${~schars}*) ]]; then
     nth=$match[2]
     test=${(L)${${match[4]##${~schars}#}%%${~schars}#}[1,3]}
     wday=${dayarr[(I)$test]}
     if (( wday )); then
       line=${line[1,$mbegin[2]-1]}${line[$mend[4]+1,-1]}
       time_found=1
       # We want weekday 0 to 6
       (( wday-- ))
       (( reldate = relative_start + reladd ))
       strftime -s year2 "%Y" $reldate
       strftime -s month2 "%m" $reldate
       # Find day of week of the first of the month we've landed on.
       strftime -rs then "%Y:%m:%d" "${year2}:${month2}:1"
       strftime -s wday2 "%w" $then
       # Calculate day of month
       (( day = 1 + (wday - wday2) + (nth - 1) * 7 ))
       (( wday < wday2 )) && (( day += 7 ))
       # whereas the day of the month calculated so far is...
       strftime -s day2 "%d" $reldate
       # so we need to compensate by...
       (( newadd += (day - day2) * daysecs ))
     fi
  fi
  if [[ $line = (#bi)${~dspat}(<->|)[[:space:]]#(w|wk|week|weekly)${~repat} ]]; then
     [[ -z $match[2] ]] && match[2]=1
     (( newadd += relsign * 7 * daysecs * ${match[2]} ))
     line=${line[1,$mbegin[2]-1]}${line[$mend[4]+1,-1]}
     time_found=1
  fi
  if [[ $line = (#bi)${~dspat}(<->|)[[:space:]]#(d|dy|day|daily)${~repat} ]]; then
     [[ -z $match[2] ]] && match[2]=1
     (( newadd += relsign * daysecs * ${match[2]} ))
     line=${line[1,$mbegin[2]-1]}${line[$mend[4]+1,-1]}
     time_found=1
  fi
  if (( relative == 2 && newadd )); then
    # You thought a day was always the same time?  Ho, ho, ho.
    # If the clocks go forward or back, we can gain or lose
    # an hour.  Check this by seeing what the hour is before
    # and after adding the number of days.  If it changes,
    # remove the difference.
    #
    # We need this correction for days (including days of a given
    # month) and weeks.
    # We don't need it for years and months because we calculated
    # those by actually looking at the calendar for a given
    # time of day, so the adjustment came out in the wash.
    # We don't need it for hours or smaller periods because
    # presumably if a user asks for something in 3 hours time
    # they don't mean 4 hours if the clocks went back and
    # 2 hours if they went forward.  At least, I think so.
    # Consider:
    #   % calendar_showdate +2d,1hr
    #   Sun Mar 25 00:37:00 GMT 2007
    #   % calendar_showdate +2d,2hr
    #   Sun Mar 25 02:37:09 BST 2007
    # At first sight that looks wrong because the clock appears
    # to jump two hours.  (Yes, it took me all of 9 seconds to
    # edit the line.)  But actually it's only jumped the hour
    # you asked for, because one is in GMT and the other in BST.
    # In principle you could say the same thing about days:
    # Sun Mar 25 00:00:00 GMT 2007  and  Mon Mar 26 01:00:00 BST 2007
    # are a day apart.  But usually if you say "same time next Tuesday"
    # you mean "when the clock says the same time, even if someone
    # has nipped in and adjusted it in the mean time", although
    # for some reason you don't usually bother saying that.
    #
    # Hope that's clear.
    strftime -s h1 "%H" $(( relative_start + reladd ))
    strftime -s h2 "%H" $(( relative_start + reladd + newadd ))
    (( hd = h2 - h1 ))
    # and of course we might go past midnight...
    if (( hd > 12 )); then
      (( hd -= 24 ))
    elif (( hd < -12 )); then
      (( hd += 24 ))
    fi
    (( newadd -= hd * 3600 ))
  fi
  (( reladd += newadd ))
  if [[ $line = (#bi)${~dspat}(<->|)[[:space:]]#(h|hr|hour|hourly)${~repat} ]]; then
     [[ -z $match[2] ]] && match[2]=1
     (( reladd += relsign * 60 * 60 * ${match[2]} ))
     line=${line[1,$mbegin[2]-1]}${line[$mend[4]+1,-1]}
     time_found=1
  fi
  if [[ $line = (#bi)${~dspat}(<->)[[:space:]]#(min|minute)${~repat} ]]; then
     (( reladd += relsign * 60 * ${match[2]} ))
     line=${line[1,$mbegin[2]-1]}${line[$mend[4]+1,-1]}
     time_found=1
  fi
  if [[ $line = (#bi)${~dspat}(<->)[[:space:]]#(s|sec|second)${~repat} ]]; then
     (( reladd += relsign * ${match[2]} ))
     line=${line[1,$mbegin[2]-1]}${line[$mend[4]+1,-1]}
     time_found=1
  fi
fi

if (( relative )); then
  # If no date was found, we're in trouble unless we found a time.
  if (( time_found )); then
    if (( anchor_end )); then
      # must be left with only separator characters
      if [[ $line != ${~schars}# ]]; then
	return 1
      fi
    fi
    # relative_start is zero if we're not using it
    (( reladd += (hour * 60 + minute) * 60 + second ))
    typeset -g REPLY
    (( REPLY = relative_start + reladd  ))
    [[ -n $setvar ]] && typeset -g REPLY2="$line$rest_line"
    return 0
  fi
  return 1
elif (( date_found == 0 )); then
  return 1
fi

if (( anchor_end )); then
  # must be left with only separator characters
  if [[ $line != ${~schars}# ]]; then
    return 1
  fi
fi

local fmt nums
if [[ -n $mname ]]; then
  fmt="%Y %b %d %H %M %S"
  nums="$year $mname $day $hour $minute $second"
else
  fmt="%Y %m %d %H %M %S"
  nums="$year $month $day $hour $minute $second"
fi

strftime -s REPLY -r $fmt $nums

[[ -n $setvar ]] && typeset -g REPLY2="$line$rest_line"

return 0
