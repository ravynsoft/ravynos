emulate -L zsh
setopt extendedglob

autoload -Uz match-words-by-style

local curcontext=":zle:$WIDGET" word done
local -a matched_words
integer count=${NUMERIC:-1}

if (( count < 0 )); then
  (( NUMERIC = -count ))
  zle backward-$WIDGET
  return
fi

while (( count-- )); do
  match-words-by-style

  word="${(j..)matched_words[4,5]}"

  if [[ -n $word ]]; then
    if [[ -n $done || $LASTWIDGET = *kill* ]]; then
      CUTBUFFER="$CUTBUFFER$word"
    else
      zle copy-region-as-kill -- $word
    fi
    RBUFFER=${(j..)matched_words[6,7]}
  else
    return 1
  fi
  done=1
done

zle -f 'kill'

return 0
