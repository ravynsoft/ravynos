# Take an expression suitable for interpolation in double quotes that
# performs a replacement on the parameter "ARG".  Replaces the
# shell argument (which may be a quoted string) under or before the
# cursor with that.  Ensure the expression is suitable quoted.
#
# For example, to uppercase the entire shell argument:
#   modify-current-argument '${(U)ARG}'
# To strip the current quoting from the word (whether backslashes or
# single, double or dollar quotes) and use single quotes instead:
#   modify-current-argument '${(qq)${(Q)ARG}}'

# Retain most options from the calling function for the eval.
# Reset some that might confuse things.
setopt localoptions noksharrays multibyte

local -a reply
integer posword poschar fromend endoffset
local REPLY REPLY2

autoload -Uz split-shell-arguments
split-shell-arguments

(( posword = REPLY, poschar = REPLY2 ))

# Can't do this unless there's some text under or left of us.
(( posword < 2 )) && return 1

# Get the index of the word we want.
if (( posword & 1 )); then
  # Odd position; need previous word.
  (( posword-- ))
  # Pretend position was just after the end of it.
  (( poschar = ${#reply[posword]} + 1 ))
fi

# Work out offset from end of string
(( fromend = $poschar - ${#reply[posword]} - 1 ))
if (( fromend >= -1 )); then
  # Cursor is near the end of the word, we'll try to keep it there.
  endoffset=1
fi

# Length of all characters before current.
# Force use of character (not index) counting and join without IFS.
integer wordoff="${(cj..)#reply[1,posword-1]}"

# Replacement for current word.  This could do anything to ${reply[posword]}.
local ARG="${reply[posword]}" repl
if [[ $1 != *ARG* ]]; then
    REPLY=
    $1 $ARG || return 1
    repl=$REPLY
else
    eval repl=\"$1\"
fi

if (( !endoffset )) && [[ ${repl[fromend,-1]} = ${ARG[fromend,-1]} ]]; then
  # If the part of the string from here to the end hasn't changed,
  # leave the cursor this distance from the end instead of the beginning.
  endoffset=1
fi

# New line:  all words before and after current word, with
# no additional spaces since we've already got the whitespace
# and the replacement word in the middle.
local left="${(j..)reply[1,posword-1]}${repl}"
local right="${(j..)reply[posword+1,-1]}"

if [[ endoffset -ne 0 && ${#repl} -ne 0 ]]; then
  # Place cursor relative to end.
  LBUFFER="$left"
  RBUFFER="$right"
  (( CURSOR += fromend ))
else
  BUFFER="$left$right"

  # Keep cursor at same position in replaced word.
  # Redundant here, but useful if $repl changes the length.
  # Limit to the next position after the end of the word.
  integer repmax=$(( ${#repl} + 1 ))
  # Remember CURSOR starts from offset 0 for some reason, so
  # subtract 1 from positions.
  (( CURSOR = wordoff + (poschar > repmax ? repmax : poschar) - 1 ))
fi
