# Select the entire word around the cursor. Intended for use as
# a vim-style text object in vi mode but with customisable
# word boundaries.
#
# For example:
#   autoload -U select-word-match
#   zle -N select-in-camel select-word-match
#   bindkey -M viopp ic select-in-camel
#   zstyle ':zle:*-camel' word-style normal-subword

emulate -L zsh
setopt extendedglob

local curcontext=:zle:$WIDGET
local -A matched_words
# Start and end of range of characters
integer pos1 pos2 num=${NUMERIC:-1}
local style word

# choose between inner word or a word style of widget
for style in $1 ${${WIDGET#*-}[1]} $KEYS[1] "i"; do
  [[ $style = [ai] ]] && break
done

autoload -Uz match-words-by-style

while (( num-- )); do
  if (( MARK > CURSOR )); then
    # if cursor is at the start of the selection, just move back a word
    match-words-by-style
    if [[ $style = i && -n $matched_words[ws-before-cursor] ]]; then
      word=$matched_words[ws-before-cursor]
    else
      word=$matched_words[word-before-cursor]$matched_words[ws-before-cursor]
    fi
    if [[ -n $word ]]; then
      (( CURSOR -= ${#word} ))
    else
      return 1
    fi
  elif (( MARK >= 0 && MARK < CURSOR )); then
    # cursor at the end, move forward a word
    (( CURSOR+1 == $#BUFFER )) && return 1
    (( CURSOR++ ))
    match-words-by-style
    if [[ -n $matched_words[ws-after-cursor] ]]; then
      if [[ $style = i ]]; then
	# just skip the whitespace
	word=$matched_words[ws-after-cursor]
      else
	# skip the whitespace plus word
	word=$matched_words[ws-after-cursor]$matched_words[word-after-cursor]
      fi
    else
      if [[ $style = i ]]; then
	# skip the word
	word=$matched_words[word-after-cursor]
      else
	# skip word and following whitespace
	word=$matched_words[word-after-cursor]$matched_words[ws-after-word]
      fi
    fi
    (( CURSOR += ${#word} - 1 ))
  else
    match-words-by-style

    if (( ${matched_words[is-word-start]} )); then
      # The word we are selecting starts at the cursor position.
      pos1=$CURSOR
    else
      # No whitespace before us, so select any wordcharacters there.
      pos1="${#matched_words[start]}"
    fi

    if [[ -n "${matched_words[ws-after-cursor]}" ]]; then
      if [[ -n "${matched_words[ws-before-cursor]}" ]] || (( CURSOR == 0 )); then
        # whitespace either side, select it
	(( pos1 = CURSOR - ${#matched_words[ws-before-cursor]} ))
	(( pos2 = CURSOR + ${#matched_words[ws-after-cursor]} ))
      else
	# There's whitespace at the cursor position, so only select
	# up to the cursor position.
	(( pos2 = CURSOR + 1 ))
      fi
    else
      # No whitespace at the cursor position, so select the
      # current character and any following wordcharacters.
      (( pos2 = CURSOR + ${#matched_words[word-after-cursor]} ))
    fi

    if [[ $style = a ]]; then
      if [[ -n "${matched_words[ws-after-cursor]}"  && ( -n "${matched_words[ws-before-cursor]}" || CURSOR -eq 0 ) ]]; then
	# in the middle of whitespace so grab a word
	if [[ -n "${matched_words[word-after-cursor]}" ]]; then
	  (( pos2 += ${#matched_words[word-after-cursor]} )) # preferably the one after
	else
	  (( pos1 -= ${#matched_words[word-before-cursor]} )) # otherwise the one before
	fi
      elif [[ -n "${matched_words[ws-after-word]}" ]]; then
	(( pos2 += ${#matched_words[ws-after-word]} ))
      elif [[ -n "${matched_words[ws-before-cursor]}" ]]; then
	# couldn't grab whitespace forwards so try backwards
	(( pos1 -= ${#matched_words[ws-before-cursor]} ))
      elif (( pos1 > 0 )); then
	# There might have been whitespace before the word
	(( CURSOR = pos1 ))
	match-words-by-style
	if [[ -n "${matched_words[ws-before-cursor]}" ]]; then
	  (( pos1 -= ${#matched_words[ws-before-cursor]} ))
	fi
      fi
    fi

    (( MARK = pos1, CURSOR = pos2-1 ))
  fi
done

if [[ $KEYMAP == vicmd ]] && (( !REGION_ACTIVE )); then
  (( CURSOR++ )) # Need to include cursor position for operators
fi
