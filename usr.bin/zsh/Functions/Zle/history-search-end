# function history-search-end {
#
# This implements functions like history-beginning-search-{back,for}ward,
# but takes the cursor to the end of the line after moving in the
# history, like history-search-{back,for}ward.  To use them:
#   zle -N history-beginning-search-backward-end history-search-end
#   zle -N history-beginning-search-forward-end history-search-end
#   bindkey '...' history-beginning-search-backward-end
#   bindkey '...' history-beginning-search-forward-end

integer cursor=$CURSOR mark=$MARK

if [[ $LASTWIDGET = history-beginning-search-*-end ]]; then
  # Last widget called set $MARK.
  CURSOR=$MARK
else
  MARK=$CURSOR
fi

if zle .${WIDGET%-end}; then
  # success, go to end of line
  zle .end-of-line
else
  # failure, restore position
  CURSOR=$cursor
  MARK=$mark
  return 1
fi
# }
