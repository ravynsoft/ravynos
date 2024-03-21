# Make line editor input go straight to the current TCP session.
# Returns when the string $TCP_TALK_ESCAPE (default :) is read on its own.
# Otherwise, $TCP_TALK_ESCAPE followed by whitespace at the start of a line
# is stripped off and the rest of the line passed to the shell.
#
# History is not currently handled, because this is difficult.

: ${TCP_TALK_ESCAPE:=:}

tcp-accept-line-or-exit() {
  emulate -L zsh
  setopt extendedglob
  local match mbegin mend

  if [[ $BUFFER = ${TCP_TALK_ESCAPE}[[:blank:]]#(#b)(*) ]]; then
    if [[ -z $match[1] ]]; then
      BUFFER=
      zle -A .accept-line accept-line
      PS1=$TCP_SAVE_PS1
      unset TCP_SAVE_PS1
      zle -I
      print '\r[Normal keyboard input restored]' >&2
    else
      BUFFER=$match[1]
    fi
    zle .accept-line
  else
    # BUGS: is deleted from the command line and doesn't appear in
    # the history.

    # The following attempt to get the BUFFER into the history falls
    # foul of the fact that we need to accept the current line first.
    # But we don't actually want to accept the current line at all.
    # print -s -r - $BUFFER

    # This is my function to send data over a TCP connection; replace
    # it with something else or nothing.
    tcp_send $BUFFER
    BUFFER=
  fi
}

TCP_SAVE_PS1=${PS1##\[T*\]}
if [[ -o prompt_subst ]]; then
  PS1="T[\$TCP_SESS]$TCP_SAVE_PS1"
else
  PS1="[T]$TCP_SAVE_PS1"
fi
zle -N tcp-accept-line-or-exit
zle -A tcp-accept-line-or-exit accept-line
