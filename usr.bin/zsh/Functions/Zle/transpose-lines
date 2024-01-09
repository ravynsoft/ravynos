# Transpose lines.  This is like in emacs: with a positive argument
# (default 1) the current line and the previous line are swapped and the
# cursor goes down one line; with a negative argument the previous two
# lines are swapped and the cursor goes up one line.

emulate -L zsh
setopt extendedglob # xtrace

local -a match mbegin mend
integer count=${NUMERIC:-1}
local init prev lline final rrline

if (( ${NUMERIC:-1} < 0 )); then
  while (( count++ )); do
    [[ $LBUFFER != (#b)(|*$'\n')([^$'\n']#$'\n')([^$'\n']#$'\n')([^$'\n']#) ]] && return 1
    
    LBUFFER=$match[1]$match[3]
    RBUFFER=$match[2]$match[4]$RBUFFER
  done
else
  while (( count-- )); do
    [[ $LBUFFER != (#b)(*)$'\n'([^$'\n']#) ]] && return 1

    prev=$match[1]
    lline=$match[2]

    if [[ $prev = (#b)(*$'\n')([^$'\n']#) ]]; then
      init=$match[1]
      prev=$match[2]
    fi

    if [[ $RBUFFER = (#b)([^$'\n']#)$'\n'(*) ]]; then
      rline=$match[1]
      final=$match[2]
      prev+=$'\n'
    else
      rline=$RBUFFER
    fi

    LBUFFER=$init$lline$rline$'\n'$prev
    RBUFFER=$final
  done
fi
