# Like up-line-or-search, but uses the whole line prefix up to the
# cursor position for searching backwards.

emulate -L zsh

typeset -g __searching __savecursor

if [[ $LBUFFER == *$'\n'* ]]; then
  zle .up-line-or-history
  __searching=''
elif [[ -n $PREBUFFER ]] && 
    zstyle -t ':zle:up-line-or-beginning-search' edit-buffer
then
  zle .push-line-or-edit
else
  [[ $LASTWIDGET = $__searching ]] && CURSOR=$__savecursor
  __savecursor=$CURSOR
  __searching=$WIDGET
  zle .history-beginning-search-backward
  zstyle -T ':zle:up-line-or-beginning-search' leave-cursor &&
      zle .end-of-line
fi
