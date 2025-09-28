# Example of a widget that takes a vi motion

# Filter part of buffer corresponding to a vi motion through an external
# program.

# To enable with vi compatible bindings use:
#   autoload -Uz vi-pipe
#   bindkey -a '!' vi-pipe

setopt localoptions noksharrays

autoload -Uz read-from-minibuffer
local _save_cut="$CUTBUFFER" REPLY

# mark this widget as a vi change so it can be repeated as a whole
zle -f vichange

# force movement to default to line mode
(( REGION_ACTIVE )) || zle -U V
# Use the standard vi-change to accept a vi motion.
zle .vi-change || return
read-from-minibuffer "!"
zle .vi-cmd-mode
local _save_cur=$CURSOR

# cut buffer contains the deleted text and can be modified
CUTBUFFER=$(eval "$REPLY" <<<"$CUTBUFFER")

# put the modified text back in position.
if [[ CURSOR -eq 0 || $BUFFER[CURSOR] = $'\n' ]]; then
  # at the beginning of a line, vi-delete won't have moved the cursor
  # back to a previous line
  zle .vi-put-before -n 1
else
  zle .vi-put-after -n 1
fi

# restore cut buffer and cursor to the start of the range
CUTBUFFER="$_save_cut" CURSOR="$_save_cur"
