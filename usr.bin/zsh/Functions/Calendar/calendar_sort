emulate -L zsh
setopt extendedglob

autoload -Uz calendar_{read,scandate,lockfiles}

local calendar line REPLY new lockfile
local -a calendar_entries
local -a times lines_sorted lines_unsorted lines_failed lockfiles
integer i

# Read the calendar file from the calendar-file style
zstyle -s ':datetime:calendar:' calendar-file calendar || calendar=~/calendar

# start of subshell for OS file locking
(
# start of block for following always to clear up lockfiles.
# Not needed but harmless if OS file locking is used.
{
  if zmodload -F zsh/system b:zsystem && zsystem supports flock &&
    zsystem flock $calendar; then
    # locked OK
    :
  else
    calendar_lockfiles $calendar || exit 1
  fi

  new=$calendar.new.$$
  calendar_read $calendar
  if [[ ${#calendar_entries} -eq 0 || \
    ( ${#calendar_entries} -eq 1 && -z $calendar_entries[1] ) ]]; then
    return 0
  fi

  for line in $calendar_entries; do
    if calendar_scandate -a $line; then
      lines_unsorted+=("${(l.16..0.)REPLY}:$line")
    else
      lines_failed+=($line)
    fi
  done

  if (( ${#lines_unsorted} )); then
    lines_sorted=(${${(o)lines_unsorted}##[0-9]##:})
  fi

  {
    for line in "${lines_failed[@]}"; do
      print "$line # BAD DATE"
    done
    (( ${#lines_sorted} )) && print -l "${lines_sorted[@]}"
  } > $new

  if [[ ! -s $new ]]; then
    print "Writing to $new failed."
    return 1
  elif (( ${#lines_failed} )); then
    print "Warning: lines with date that couldn't be parsed.
Output (with unparseable dates marked) left in $new"
    return 1
  fi

  if ! mv $calendar $calendar.old; then
    print "Couldn't back-up $calendar to $calendar.old.
New calendar left in $new"
    return 1
  fi
  if ! mv $new $calendar; then
    print "Failed to rename $new to $calendar.
Old calendar left in $calendar.old"
    return 1
  fi

  print "Old calendar left in $calendar.old"
} always {
  (( ${#lockfiles} )) && rm -rf $lockfiles
}
)
