# Transpose words, matching the words using match-words-by-style, q.v.
# The group of word characters preceding the cursor (not necessarily
# immediately) are transposed with the group of word characters following
# the cursor (again, not necessarily immediately).
#
# Note the style skip-chars, used in the context of the current widget.
# This gives a number of character starting from the cursor position
# which are never considered part of a word and hence are always left
# alone.  The default is 0 and typically the only useful alternative
# is one.  This would have the effect that `fooXbar' with the cursor
# on X would be turned into `barXfoo' with the cursor still on the X,
# regardless of what the character X is.

emulate -L zsh
autoload -Uz match-words-by-style

local curcontext=":zle:$WIDGET"
local -a matched_words
integer count=${NUMERIC:-1} neg

(( count < 0 )) && (( count = -count, neg = 1 ))

if [[ $WIDGET == transpose-words ]]; then
  # default is to be a drop-in replacement, check styles for change
  zstyle -m $curcontext skip-chars \* ||
  zstyle -m $curcontext word-style '*subword*' ||
  { [[ $LBUFFER[-1] != [[:space:]] && $RBUFFER[1] != [[:space:]] ||
       -z ${RBUFFER//[[:space:]]/} ]] && zle backward-word }
fi

while (( count-- > 0 )); do
    match-words-by-style

    [[ -z "$matched_words[2]$matched_words[5]" ]] && return 1

    if (( neg )); then
	LBUFFER="$matched_words[1]"
	RBUFFER="$matched_words[5]${(j..)matched_words[3,4]}\
$matched_words[2]${(j..)matched_words[6,7]}"
    else
	LBUFFER="$matched_words[1]$matched_words[5]${(j..)matched_words[3,4]}\
$matched_words[2]"
	RBUFFER="${(j..)matched_words[6,7]}"
    fi

done

return 0
