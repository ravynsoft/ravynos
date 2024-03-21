# Prompt for an search in the history for a pattern.
# Patterns to search are standard zsh patterns, but may include
# ^ at the start or $ at the end to anchor the pattern to the
# start or end of the history entry respectively.
#
# To search backwards, create a widget history-pattern-search-backward:
#   zle -N history-pattern-search-backward history-pattern-search
# and to search forwards, create history-pattern-search-forward
#   zle -N history-pattern-search-forward history-pattern-search

# Use extended globbing by default.
emulate -L zsh
setopt extendedglob

# Load required features.
autoload -Uz read-from-minibuffer
zmodload -i zsh/parameter

local REPLY dir new
integer i
local -a found match mbegin mend

# Decide if we are searching backwards or forwards.
if [[ $WIDGET = *forward* ]]; then
  dir="forw"
else
  dir="rev"
fi

# Read pattern.  Prompt could be made customisable.
read-from-minibuffer "pat ($dir): " $_last_history_pattern_search

_last_history_pattern_search=$REPLY

# Abort if bad status or nothing entered
[[ $? -ne 0 || -z $REPLY ]] && return 0

# Handle start-of-line anchor.
if [[ $REPLY = \^* ]]; then
  REPLY=$REPLY[2,-1]
else
  REPLY="*$REPLY"
fi

# Handle end-of-line anchor.
if [[ $REPLY = *\$ ]]; then
  REPLY=$REPLY[1,-2]
else
  REPLY="$REPLY*"
fi

# Search history for pattern.
# As $history is an associative array we can get all matches.
found=(${(kon)history[(R)$REPLY]})

if [[ $dir = forw ]]; then
  # Searching forward.  Look back through matches until we
  # get back to the current history number.
  for (( i = ${#found}; i >= 1; i-- )); do
    (( $found[$i] <= HISTNO )) && break
    new=$found[$i]
  done
else
  # Searching backward.  Look forward through matches until we
  # reach the current history number.
  for (( i = 1; i <= ${#found}; i++ )); do
    (( $found[$i] >= HISTNO )) && break
    new=$found[$i]
  done
fi

if [[ -n $new ]]; then
  # Match found.  Move to line.
  HISTNO=$new
  if [[ $REPLY = *\* && $history[$new] = (#b)(${~REPLY[1,-2]})* ]]; then
    # If not anchored to the end, move to the end of the pattern
    # we were searching for.
    CURSOR=$mend[1]
  fi
  return 0
else
  return 1
fi
