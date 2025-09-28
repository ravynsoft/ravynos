# Delete the entire word around the cursor.  Does not handle
# a prefix argument; either the cursor is in the word or it isn't.
# The word may be just before the cursor, e.g.
#   print this is a line
#             ^ here
# and then the word before (i.e. `this') will be deleted.
#
# If the widget has the name `kill' in, the text deleted will be
# saved for future yanking in the normal way.

emulate -L zsh
setopt extendedglob

local curcontext=:zle:$WIDGET
local -A matched_words
# Start and end of range of characters to remove.
integer pos1 pos2

autoload -Uz match-words-by-style
match-words-by-style

if (( ${matched_words[is-word-start]} )); then
    # The word we are deleting starts at the cursor position.
    pos1=$CURSOR
else
    # Not, so delete any wordcharacters before, too
    pos1="${#matched_words[start]}"
fi

if [[ -n "${matched_words[ws-after-cursor]}" ]]; then
    # There's whitespace at the cursor position, so only delete
    # up to the cursor position.
    (( pos2 = CURSOR + 1 ))
else
    # No whitespace at the cursor position, so delete the
    # current character and any following wordcharacters.
    (( pos2 = CURSOR + ${#matched_words[word-after-cursor]} + 1 ))
fi

# Move the cursor then delete the block in one go for the
# purpose of undoing (and yanking, if appropriate).
(( CURSOR = pos1 ))

# If the widget name includes the word `kill', the removed
# text goes into the cutbuffer in the standard way.
if [[ $WIDGET = *kill* ]]; then
  local word="${BUFFER[pos1+1,pos2-1]}"
  if [[ $LASTWIDGET = *kill* ]]; then
    CUTBUFFER="$CUTBUFFER$word"
  else
    zle copy-region-as-kill -- "$word"
  fi
fi
BUFFER="${BUFFER[1,pos1]}${BUFFER[pos2,-1]}"
