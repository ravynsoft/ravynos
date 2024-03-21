# Split a command line into shell arguments and whitespace in $reply.
# Odd elements (starting from 1) are whitespace, even elements
# are shell arguments (possibly quoted strings).  Whitespace at
# start and end is always included in the array but may be an empty string.
# $REPLY holds NO_KSH_ARRAYS index of current word in $reply.
# $REPLY2 holds NO_KSH_ARRAYS index of current character in current word.
# Hence ${reply[$REPLY][$REPLY2]} is the character under the cursor.
#
# reply, REPLY, REPLY2 should therefore be local to the enclosing function.

emulate -L zsh
setopt extendedglob

local -a bufwords lbufwords
local word
integer pos=1 cpos=$((CURSOR+1)) opos iword ichar

bufwords=(${(Z+n+)BUFFER})

typeset -ga reply
reply=()
while [[ ${BUFFER[pos]} = [[:space:]] ]]; do
  (( pos++ ))
done
reply+=${BUFFER[1,pos-1]}
(( cpos < pos )) && (( iword = 1, ichar = cpos ))

for word in "${bufwords[@]}"; do
  (( opos = pos ))
  (( pos += ${#word} ))
  reply+=("$word")
  if (( iword == 0  &&  cpos < pos )); then
    (( iword = ${#reply} ))
    (( ichar = cpos - opos + 1 ))
  fi

  (( opos = pos ))
  while [[ ${BUFFER[pos]} = [[:space:]] ]]; do
    (( pos++ ))
  done
  reply+=("${BUFFER[opos,pos-1]}")
  if (( iword == 0  &&  cpos < pos )); then
    (( iword = ${#reply} ))
    (( ichar = cpos - opos + 1 ))
  fi
done

typeset -g REPLY REPLY2
if (( iword == 0 )); then
  # At the end of the line, so off the indexable positions
  # (but still a valid cursor position).
  (( REPLY = ${#reply} ))
  (( REPLY2 = 1 ))
else
  (( REPLY = iword ))
  (( REPLY2 = ichar ))
fi
