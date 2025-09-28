# Utility for "calendar" to read entries into the array calendar_entries.
# This should be local to the caller.
# The only argument is the file to read.  We expect options etc. to
# be correct.
#
# This is based on Emacs calendar syntax, which has two implications:
#  - Lines beginning with whitespace are continuation lines.
#    Hence we have to read the entire file first to determine entries.
#  - Lines beginning with "&" are inhibited from producing marks in
#    Emacs calendar window.  This is irrelevant to us, so we
#    we simply remove leading ampersands.  This is necessary since
#    we expect the date to start at the beginning of the line.
#
# TODO: Emacs has some special handling for entries where the first line
# has only the date and continuation lines indicate times.  Actually,
# it doesn't parse the times as far as I can see, but if we want to
# handle that format sensibly we would need to here.  It could
# be tricky to get right.

local calendar=$1 line
local -a lines

lines=(${(f)"$(<$calendar)"})

calendar_entries=()
# ignore blank lines
for line in $lines; do
  if [[ $line = [[:space:]]* ]]; then
    if (( ${#calendar_entries} )); then
      calendar_entries[-1]+=$'\n'$line
    fi
  else
    calendar_entries+=(${line##\&})
  fi
done
