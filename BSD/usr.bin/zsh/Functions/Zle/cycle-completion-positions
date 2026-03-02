# This may be called after a completion that inserted the unambiguous
# (i.e. non-menu- and non-single-match-) string into the command line.
# If there are multiple positions in the string with missing or differing
# characters, repeatedly calling this widget cycles between all these
# positions.

emulate -L zsh
setopt extendedglob

local p="$_lastcomp[insert_positions]"

if [[ $p = ((#s)|*:)${CURSOR}:* ]]; then
  CURSOR=${${p#(|*:)${CURSOR}:}%%:*}
elif [[ -n $p ]]; then
  CURSOR=${p%%:*}
fi
