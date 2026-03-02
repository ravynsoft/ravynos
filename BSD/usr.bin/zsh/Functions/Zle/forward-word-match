emulate -L zsh
setopt extendedglob

autoload -Uz match-words-by-style

local curcontext=":zle:$WIDGET" word
local -a matched_words
integer count=${NUMERIC:-1}

if (( count < 0 )); then
  (( NUMERIC = -count ))
  zle ${WIDGET/forward/backward}
  return
fi

while (( count-- )); do
  match-words-by-style
 
  if zstyle -t $curcontext skip-whitespace-first; then
    # Standard non-zsh behaviour: skip leading whitespace and the word.
    word=$matched_words[4]$matched_words[5]
  else
    # Traditional zsh behaviour.
    # For some reason forward-word doesn't work like the other word
    # commands; it skips whitespace only after any matched word
    # characters.
    if [[ -n $matched_words[4] ]]; then
      # just skip the whitespace
      word=$matched_words[4]
    else
      # skip the word and trailing whitespace
      word=$matched_words[5]$matched_words[6]
    fi
  fi

  if [[ -n $word ]]; then
    (( CURSOR += ${#word} ))
  else
    return 1
  fi
done

return 0
