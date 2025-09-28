#autoload

# This widget uses the completion system to double-quote the current word
# if it is not already quoted, then attempts to complete normally.  If the
# normal completion fails, the quotes are removed again.
#
# To use it:
#   autoload -Uz quote-and-complete-word
#   zle -N quote-and-complete-word
#   bindkey '\t' quote-and-complete-word
#
# BUG: The "undo" mechanism is confused by multiple calls to completion
# widgets from the same normal widget.

# Note: It's important that this function's name ends in "complete-word".
# The _oldlist completer does nothing unless the widget has that suffix.

quote-and-complete-word () {
    setopt localoptions unset noshwordsplit noksharrays
    local lbuf=$LBUFFER rbuf=$RBUFFER last=$LASTWIDGET
    if [[ $last != $WIDGET ]]
    then
        local oldcontext=$curcontext
        local curcontext="${WIDGET}:${${curcontext:-:::}#*:}"
        zle complete-word
        curcontext=$oldcontext
    fi
    zle complete-word
    local ret=$?
    if [[ _lastcomp[nmatches] -eq 0 && $last != $WIDGET ]]
    then
        LBUFFER=$lbuf RBUFFER=$rbuf
    fi
    return ret
}

_force_quote () {
    [[ -z $compstate[quoting] ]] &&
    compstate[to_end]='' &&
    compadd -U -S "$SUFFIX" -I "$ISUFFIX"\" -i \""$IPREFIX" "${(Q)PREFIX}"
}
zstyle ':completion:quote-and-complete-word:*' completer _force_quote

# Handle zsh autoloading conventions

[[ -o kshautoload ]] || quote-and-complete-word "$@"
