# Match the age of a file, for use as a glob qualifier.  Can
# take one or two arguments, which can be supplied by one of two
# ways (always the same for both arguments):
#
#   print *(e:age 2006/10/04 2006/10/09:)
#
# Match all files modified between the start of those dates.
#
#   print *(e:age 2006/10/04:)
#
# Match all files modified on that date.  If the second argument is
# omitted it is taken to be exactly 24 hours after the first argument
# (even if the first argument contains a time).
#
#   print *(e-age 2006/10/04:10:15 2006/10/04:10:45-)
#
# Supply times.  All the time and formats handled by calendar_scandate
# are allowed, but whitespace must be quoted to ensure age receives
# the correct arguments.
#
#   AGEREF=2006/10/04:10:15
#   AGEREF2=2006/10/04:10:45
#   print *(+age)
#
# The same example using the other form of argument passing.  The
# dates stay in effect until unset, but will be overridden if
# any argument is passed in the first format.

emulate -L zsh

zmodload -F zsh/stat b:zstat
zmodload -i zsh/parameter

autoload -Uz calendar_scandate

local timefmt
local -a vals tmp

[[ -e $REPLY ]] || return 1
zstat -A vals +mtime -- $REPLY || return 1

if (( $# >= 1 )); then
  if [[ $1 = :* ]]; then
      if (( $# > 1 )); then
	  timefmt="%Y/%m/%d:%H:%M:%S"
      else
	  timefmt="%Y/%m/%d"
      fi
      zstat -A tmp -F $timefmt +mtime -- ${1#:} || return 1
    local AGEREF=$tmp[1]
  else
    local AGEREF=$1
  fi
  # if 1 argument given, never use globally defined AGEREF2
  if [[ $2 = :* ]]; then
    zstat -A tmp -F "%Y/%m/%d:%H:%M:%S" +mtime -- ${2#:} || return 1
    local AGEREF2=$tmp[1]
  else
    local AGEREF2=$2
  fi
fi

integer mtime=$vals[1] date1 date2
local REPLY REPLY2

# allow a time only (meaning today)
if calendar_scandate -t $AGEREF; then
  date1=$REPLY

  if [[ -n $AGEREF2 ]]; then
    if [[ $AGEREF2 = +* ]]; then
      calendar_scandate -rt $AGEREF2[2,-1] || return 1
      (( date2 = date1 + REPLY ))
    else
      calendar_scandate -t $AGEREF2 || return 1
      date2=$REPLY
    fi
  else
    (( date2 = date1 + 24 * 60 * 60 ))
  fi

  (( date1 <= mtime && mtime <= date2 ))
else
  return 1
fi
