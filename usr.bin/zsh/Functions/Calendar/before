# Glob qualifier function, e.g
#
# print *(e:after 2014/08/01:)
# print *(e-after today:12:00-)
#
# If named before:
# Match files modified before a given time.
#
# If named after:
# Match files modified after a given time.  Use as glob qualifier.
# N.B.: "after" actually includes the given time as it is to second
# precision (it would be inconvenient to exclude the first second of a date).
# It should therefore more logically be called "from", but that's a less
# obvious name.
#
# File to test is in $REPLY.
#
# Similar to age, but only takes at most one data, which is
# compared directly with the current time.

emulate -L zsh

zmodload -F zsh/stat b:zstat
zmodload -i zsh/parameter

autoload -Uz calendar_scandate

local timefmt
local -a vals tmp

[[ -e $REPLY ]] || return 1
zstat -A vals +mtime -- $REPLY || return 1

if (( $# == 1 )); then
  if [[ $1 = :* ]]; then
    timefmt="%Y/%m/%d:%H:%M:%S"
    zstat -A tmp -F $timefmt +mtime -- ${1#:} || return 1
    local AGEREF=$tmp[1]
  else
    local AGEREF=$1
  fi
fi

integer mtime=$vals[1] date1 date2
local REPLY REPLY2

# allow a time only (meaning today)
if calendar_scandate -t $AGEREF; then
  date1=$REPLY

  case $0 in
    (after)
    (( mtime >= date1 ))
    ;;

    (before)
    (( mtime < date1 ))
    ;;

    (*)
    print "$0: must be named 'after' or 'before'" >&2
    return 1
    ;;
  esac
else
  return 1
fi
