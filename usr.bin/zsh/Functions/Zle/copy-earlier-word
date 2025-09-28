# Copy the word before the one you last copied --- call repeatedly
# to cycle through the list of words on the history line.
#
# Words in combination with insert-last-word to use the line reached,
# and start from the word before last.  Otherwise, it will operate on
# the current line.

emulate -L zsh
setopt typesetsilent

typeset -g __copyword
if (( ${NUMERIC:-0} )); then
  # 1 means last word, 2 second last, etc.
  (( __copyword = ${NUMERIC:-0} ))
  zstyle -s :$WIDGET widget __copywidget
elif [[ -n $__copyword && $WIDGET = $LASTWIDGET ]]; then
  (( __copyword-- ))
elif [[ $LASTWIDGET = *insert-last-word ]]; then
  __copyword=-2
  typeset -g __copywidget=$LASTWIDGET
else
  __copyword=-1
  zstyle -s :$WIDGET widget __copywidget
fi

zle ${__copywidget:-.insert-last-word} 0 $__copyword
