# Replace an argument to a command, delimited by normal shell syntax.
# Prompts for the replacement.
# With no numeric argument, replace the current argument.
# With a numeric argument, replace that argument: 0 = command word,
# as in history expansion.
# If editing buffer is empty, use previous history line.

autoload -Uz split-shell-arguments read-from-minibuffer

if (( ${#BUFFER} == 0 )); then
  (( HISTNO-- ))
  CURSOR=${#BUFFER}
fi

local widget=$WIDGET numeric
integer cursor=CURSOR
if (( ${+NUMERIC} )); then
  numeric=$NUMERIC
fi
local reply REPLY REPLY2
integer index
split-shell-arguments

if [[ -n $numeric ]]; then
  if (( numeric < 0 )); then
    (( index = ${#reply} - 1 + 2*(numeric+1) ))
  else
    (( index = 2 + 2*numeric ))
  fi
else
  (( index = REPLY & ~1 ))
fi

local edit
if [[ $widget = *edit* ]]; then
  edit=$reply[$index]
fi
read-from-minibuffer "Replace $reply[$index] with: " $edit || return 1

integer diff=$(( ${#REPLY} - ${#reply[$index]} ))
reply[$index]=$REPLY

BUFFER=${(j..)reply}
if (( cursor > REPLY2 )); then
  (( CURSOR = cursor + diff ))
else
  (( CURSOR = REPLY2 ))
fi
