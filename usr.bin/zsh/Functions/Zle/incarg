# Shell function to increment an integer either under the cursor or just
# to the left of it.  Use
#   autoload -Uz incarg
#   zle -N incarg
#   bindkey "..." incarg
# to define it.  For example,
#   echo 41
#        ^^^ cursor anywhere here
# with incarg gives
#   echo 42
# with the cursor in the same place.
#
# A numeric argument gives a number other than 1 to add (may be negative).
# If you're going to do it a lot with one particular number, you can set
# the parameter incarg to that number (a numeric argument still takes
# precedence).

emulate -L zsh
setopt extendedglob

local rrest lrest num

rrest=${RBUFFER##[0-9]#}
if [[ $RBUFFER = [0-9]* ]]; then
  if [[ -z $rrest ]]; then
    num=$RBUFFER
  else
    num=${RBUFFER[1,-$#rrest-1]}
  fi
fi

lrest=${LBUFFER%%[0-9]#}
if [[ $LBUFFER = *[0-9] ]]; then
  if [[ -z $lrest ]]; then
    num="$LBUFFER$num"
  else
    num="${LBUFFER[$#lrest+1,-1]}$num"
  fi
fi

[[ -n $num ]] && (( num += ${NUMERIC:-${incarg:-1}} ))

BUFFER="$lrest$num$rrest"
