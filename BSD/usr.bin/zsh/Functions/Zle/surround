# Implementation of some functionality of the popular vim surround plugin.
# Allows "surroundings" to be changes: parentheses, brackets and quotes.

# To use
#   autoload -Uz surround
#   zle -N delete-surround surround
#   zle -N add-surround surround
#   zle -N change-surround surround
#   bindkey -a cs change-surround
#   bindkey -a ds delete-surround
#   bindkey -a ys add-surround
#   bindkey -M visual S add-surround
#
#  This doesn't allow yss to operate on a line but VS will work

setopt localoptions noksharrays
autoload -Uz select-quoted select-bracketed
local before after
local -A matching
matching=( \( \) \{ \} \< \> \[ \] )

zle -f vichange
case $WIDGET in
  change-*)
    local MARK="$MARK" CURSOR="$CURSOR" call
    read -k 1 before
    if [[ ${(kvj::)matching} = *$before* ]]; then
      call=select-bracketed
    else
      call=select-quoted
    fi
    read -k 1 after
    $call "a$before" || return 1
    before="$after"
    if [[ -n $matching[$before] ]]; then
      after=" $matching[$before]"
      before+=' '
    elif [[ -n $matching[(r)[$before:q]] ]]; then
      before="${(k)matching[(r)[$before:q]]}"
    fi
    BUFFER[CURSOR]="$after"
    BUFFER[MARK+1]="$before"
    CURSOR=MARK
  ;;
  delete-*)
    local MARK="$MARK" CURSOR="$CURSOR" call
    read -k 1 before
    if [[ ${(kvj::)matching} = *$before* ]]; then
      call=select-bracketed
    else
      call=select-quoted
    fi
    if $call "a$before"; then
      BUFFER[CURSOR]=''
      BUFFER[MARK+1]=''
      CURSOR=MARK
    fi
  ;;
  add-*)
    local save_cut="$CUTBUFFER"
    zle .vi-change || return
    local save_cur="$CURSOR"
    zle .vi-cmd-mode
    read -k 1 before
    after="$before"
    if [[ -n $matching[$before] ]]; then
      after=" $matching[$before]"
      before+=' '
    elif [[ -n $matching[(r)[$before:q]] ]]; then
      before="${(k)matching[(r)[$before:q]]}"
    fi
    CUTBUFFER="$before$CUTBUFFER$after"
    if [[ CURSOR -eq 0 || $BUFFER[CURSOR] = $'\n' ]]; then
      zle .vi-put-before -n 1
    else
      zle .vi-put-after -n 1
    fi
    CUTBUFFER="$save_cut" CURSOR="$save_cur"
  ;;
esac
