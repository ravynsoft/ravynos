emulate -L zsh
setopt extendedglob

autoload -Uz read-from-minibuffer replace-string-again

local p1  p2
integer changeno=$UNDO_CHANGE_NO

{

if [[ -n $_replace_string_src ]]; then
  p1="[$_replace_string_src -> $_replace_string_rep]"$'\n'
fi

p1+="Replace: "
p2="   with: "

# Saving curwidget is necessary to avoid the widget name being overwritten.
local REPLY previous curwidget=$WIDGET

if (( ${+NUMERIC} )); then
  (( $NUMERIC > 0 )) && previous=1
else
  zstyle -t ":zle:$WIDGET" edit-previous && previous=1
fi

read-from-minibuffer $p1 ${previous:+$_replace_string_src} || return 1
if [[ -n $REPLY ]]; then
  typeset -g _replace_string_src=$REPLY

  read-from-minibuffer "$p1$_replace_string_src$p2" \
    ${previous:+$_replace_string_rep} || return 1
  typeset -g _replace_string_rep=$REPLY
fi

} always {
  # Undo back to the original line; we don't want the
  # undo history of editing the strings left.
  zle undo $changeno
}

replace-string-again $curwidget
