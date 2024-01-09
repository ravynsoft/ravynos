# Menu-driven alternative to history-beginning-search-backward.
# As it uses a menu there is no sense of "forward" or "backward", however;
# the entire history is searched.
#
# Configuration:
#   autoload -Uz history-beginning-search-menu
#   zle -N history-beginning-search-menu
#   bindkey '\eP' history-beginning-search-menu
#
# Example:
#   % /bin/su<ESC-P>
#   Enter digit:
#   1 /bin/su -c 'make install'            4 /bin/su - perforce
#   2 /bin/su                              5 /bin/su -c
#   3 /bin/su -c 'chown pws:pws **/*(u0)'
#
# Typing "1" expands the line to
#   % /bin/su -c 'make install'
#
# With a prefix argument, the search is not anchored to the beginning,
# so for example "/su" could expand to "p4 files //depot/support/..."
#
# If this is bound to a widget containing "-end", e.g.
#   zle -N history-beginning-search-menu-end history-beginning-search-menu
# then the cursor is put at the end of the line, else it is left
# after the matched characters.
#
# If this is bound to a widget containing "-space", then any space in
# the line so far is matched as a wildcard.  (This means putting a space
# at the start of the line is equivalent to specifying a prefix
# argument.)

emulate -L zsh
setopt extendedglob

zmodload -i zsh/parameter

local -aU matches
local -a display

local search=$LBUFFER MATCH MBEGIN MEND

search=${search//(#m)[\][()\\*?#<>~^]/\\$MATCH}
if [[ $WIDGET = *-space* ]]; then
  # We need to quote metacharacters in the search string
  # since they are otherwise active in the reverse subscript.
  # We need to avoid quoting other characters since they aren't
  # and just stay quoted, rather annoyingly.
  search=${search// /*}
fi

if (( ${+NUMERIC} )); then
  matches=(${(o)history[(R)*${search}*]})
else
  matches=(${(o)history[(R)${search}*]})
fi

# Filter out any match that's the same as the original.
# Note this isn't a pattern this time.
matches=(${matches:#${LBUFFER}})

integer n=${#matches}
integer width=${#n}

(( n == 0 )) && return 1

# Hey, this works...
integer i
display=(${matches/(#m)*/${(l.$width..0.):-$((++i))} $MATCH})
zle -R "Enter digit${${width##1}:+s}:" $display

integer i
local char chars

# Hmmm... this isn't great.  The only way of clearing the display
# appears to be to overwrite it completely.  I think that's because
# displaying strings in this way doesn't set the completion list
# properly.
display=(${display//?/ })

# Abort on first non-digit entry instead of requiring all
# characters to be typed (as "read -k$width chars" would do).
for (( i = 0; i < $width; i++ )); do
  read -k char
  if [[ $char != [[:digit:]] ]]; then
    zle -R '' $display
    return 1
  fi
  chars+=$char
done

if [[ $chars -eq 0 || $chars -gt $n ]]; then
  zle -R '' $display
  return 1
fi

integer newcursor
if [[ $WIDGET != *-end* ]]; then
  if (( ${+NUMERIC} )); then
    # Advance cursor so that it's still after the string typed
    local -a match mbegin mend
    if [[ $matches[$chars] = (#b)(*${LBUFFER})* ]]; then
       newcursor=${#match[1]}
    fi
  else
    # Maintain cursor
    newcursor=$CURSOR
  fi
fi

# Find the history lines that contain the matched string and
# go to the last one.  This allows accept-line-and-down-history etc.
# to work.
local -a lines
local matchq=${matches[$chars]//(#m)[\][|()\\*?#<>~^]/\\$MATCH}
lines=(${(kon)history[(R)$matchq]})
HISTNO=$lines[-1]

if (( newcursor )); then
  CURSOR=$newcursor
elif [[ $WIDGET = *-end* ]]; then
  CURSOR=${#BUFFER}
fi

zle -R '' $display
