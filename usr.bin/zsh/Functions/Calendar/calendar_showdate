emulate -L zsh
setopt extendedglob
zmodload -i zsh/datetime

local optm datefmt opt
integer optr replyset

zstyle -s ':datetime:calendar_showdate:' date-format datefmt ||
  datefmt="%a %b %d %H:%M:%S %Z %Y"

# Memo to myself: both + and - are documented as giving relative
# times, so it's not a good idea to rewrite this to use getopts.
# We need to detect the small number of options this can actually
# handle.
while [[ $1 = -r || $1 = -- || $1 = -f* ]]; do
  case $1 in
    (-r)
    shift
    REPLY=0
    optr=1
    ;;

    (-f*)
    if [[ $1 = -f?* ]]; then
      datefmt=$1[3,-1]
      shift
    else
      shift
      if [[ -z $1 || $1 != *%* ]]; then
	print "$0: -f requires a date/time specification" >&2
	return 1
      fi
      datefmt=$1
      shift
    fi
    ;;

    (--)
    shift
    break
    ;;
  esac
done

(( optr )) || local REPLY

if (( ! $# )); then
  print "Usage: $0 datespec [ ... ]" >&2
  return 1
fi

while (( $# )); do
  optm=
  if [[ $1 = [-+]* ]]; then
    # relative
    [[ $1 = -* ]] && optm=-m
    1=${1[2,-1]}
    # if this is the first argument, use current time
    # don't make assumptions about type of reply in case global
    if (( ! replyset )); then
      REPLY=$EPOCHSECONDS
      replyset=1
    fi
  fi

  if (( replyset )); then
    calendar_scandate $optm -R $REPLY -aA $1 || return 1
    replyset=1
  else
    calendar_scandate -aA $1 || return 1
  fi

  shift
done

(( optr )) && return
strftime $datefmt $REPLY
