emulate -L zsh
setopt extendedglob

autoload -Uz match-words-by-style

local curcontext=":zle:$WIDGET" word
local -a matched_words
integer count=${NUMERIC:-1}

while (( count-- > 0 )); do
    match-words-by-style

    word=${(j..)matched_words[4,5]}

    if [[ -n word ]]; then
	LBUFFER+=${(L)word}
	RBUFFER=${(j..)matched_words[6,7]}
    else
	return 1
    fi
done

return 0
