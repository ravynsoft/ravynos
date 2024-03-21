# Like down-line-or-search, but uses the whole line prefix up to the
# cursor position for searching forwards.

emulate -L zsh

typeset -g __searching __savecursor

if [[ ${+NUMERIC} -eq 0 &&
    ( $LASTWIDGET = $__searching || $RBUFFER != *$'\n'* ) ]]
then
  [[ $LASTWIDGET = $__searching ]] && CURSOR=$__savecursor
  __searching=$WIDGET
  __savecursor=$CURSOR
  if zle .history-beginning-search-forward; then
    [[ $RBUFFER = *$'\n'* ]] || 
	zstyle -T ':zle:down-line-or-beginning-search' leave-cursor &&
	zle .end-of-line
    return
  fi
  [[ $RBUFFER = *$'\n'* ]] || return
fi
__searching=''
zle .down-line-or-history
