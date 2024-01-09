emulate -L zsh
setopt extendedglob

autoload match-words-by-style

local curcontext=":zle:$WIDGET" word done
local -a matched_words
integer count=${NUMERIC:-1}

if (( count < 0 )); then
  (( NUMERIC = -count ))
  zle ${WIDGET##backward-}
  return
fi

while (( count-- )); do

  match-words-by-style

  word="$matched_words[2]$matched_words[3]"

  if [[ -n $word ]]; then
    if [[ -n $done || $LASTWIDGET = *kill* ]]; then
      CUTBUFFER="$word$CUTBUFFER"
    else
      zle copy-region-as-kill -- "$word"
    fi
    LBUFFER=$matched_words[1]
  else
    return 1
  fi
  done=1
done

zle -f 'kill'

return 0
