emulate -L zsh
setopt extendedglob

autoload match-words-by-style

local curcontext=":zle:$WIDGET" word
local -a matched_words
integer count=${NUMERIC:-1}

if (( count < 0 )); then
    (( NUMERIC = - count ))
    zle ${WIDGET/backward/forward}
    return
fi

while (( count-- )); do

    match-words-by-style

    word=$matched_words[2]$matched_words[3]

    if [[ -n $word ]]; then
	(( CURSOR -= ${#word} ))
    else
	return 1
    fi
done

return 0
