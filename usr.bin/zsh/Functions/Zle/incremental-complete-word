# Autoload this function, run `zle -N <func-name>' and bind <func-name>
# to a key.


# This allows incremental completion of a word.  After starting this
# command, a list of completion choices can be shown after every character
# you type, which you can delete with ^h or DEL.  RET will accept the
# completion so far.  You can hit TAB to do normal completion, ^g to
# abort back to the state when you started, and ^d to list the matches.
#
# This works only with the new function based completion system.

# Recommended settings:
#   zstyle ':completion:incremental:*' completer _complete _ignored
#   zstyle :incremental stop-keys $'[\e\C-b\C-f\C-n\C-p\C-u-\C-x]'

# BUGS:
# The _oldlist completer breaks incremental completion.  Use a context-
# specific completer zstyle as shown above to disable the _oldlist
# completer in this function.

# The main widget function.

incremental-complete-word() {
  emulate -L zsh
  unsetopt autolist menucomplete automenu # doesn't work well

  local key lbuf="$LBUFFER" rbuf="$RBUFFER" pmpt pstr word
  local lastl lastr wid twid num post toolong
  local curcontext="${curcontext}" stop brk

  [[ -z "$curcontext" ]] && curcontext=:::
  curcontext="incremental:${curcontext#*:}"

  zstyle -s ":incremental" prompt pmpt ||
    pmpt='incremental (%c): %u%s  %l'
  zstyle -s ":incremental" stop-keys stop
  zstyle -s ":incremental" break-keys brk

  if zstyle -t ":incremental" list; then
    wid=list-choices
    post=( icw-list-helper )
  else
    wid=complete-word
    post=()
  fi

  comppostfuncs=( "$post[@]" )
  zle $wid "$@"
  LBUFFER="$lbuf"
  RBUFFER="$rbuf"
  num=$_lastcomp[nmatches]
  if (( ! num )); then
    word=''
    state='-no match-'
  elif [[ "${LBUFFER}${RBUFFER}" = *${_lastcomp[unambiguous]}* ]]; then
    word=''
    state='-no prefix-'
  else
    word="${_lastcomp[unambiguous]}"
    state=''
  fi
  zformat -f pstr "$pmpt" "u:${word}" "s:$state" "n:$num" \
                          "l:$toolong" "c:${_lastcomp[completer]}"
  zle -R "$pstr"
  read -k key

  while [[ '#key' -ne '#\\r' && '#key' -ne '#\\n' &&
           '#key' -ne '#\\C-g' ]]; do
    twid=$wid
    if [[ "$key" = ${~stop} ]]; then
      zle -U - "$key"
      return
    elif [[ "$key" = ${~brk} ]]; then
      return
    elif [[ '#key' -eq '#\\C-h' || '#key' -eq '#\\C-?' ]]; then
      [[ $#LBUFFER -gt $#l ]] && LBUFFER="$LBUFFER[1,-2]"
    elif [[ '#key' -eq '#\\t' ]]; then
      zle complete-word "$@"
      lbuf="$LBUFFER"
      rbuf="$RBUFFER"
    elif [[ '#key' -eq '#\\C-d' ]]; then
      twid=list-choices
    else
      LBUFFER="$LBUFFER$key"
    fi
    if (( ! PENDING )); then
      lastl="$LBUFFER"
      lastr="$RBUFFER"
      [[ "$twid" = "$wid" ]] && comppostfuncs=( "$post[@]" )
      toolong=''
      zle $twid "$@"
      LBUFFER="$lastl"
      RBUFFER="$lastr"
      num=$_lastcomp[nmatches]
      if (( ! num )); then
        word=''
        state='-no match-'
      elif [[ "${LBUFFER}${RBUFFER}" = *${_lastcomp[unambiguous]}* ]]; then
        word=''
        state='-no prefix-'
      else
        word="${_lastcomp[unambiguous]}"
        state=''
      fi
      zformat -f pstr "$pmpt" "u:${word}" "s:$state" "n:$num" \
                              "l:$toolong" "c:${_lastcomp[completer]}"
      zle -R "$pstr"
    else
      zle -R
    fi
    read -k key
  done

  if [[ '#key' -eq '#\\C-g' ]]; then
    LBUFFER="$lbuf"
    RBUFFER="$rbuf"
  fi
  zle -Rc
}

# Helper function used as a completion post-function used to make sure that
# the list of matches in only shown if it fits on the screen.

icw-list-helper() {

  # +1 for the status line we will add...

  if [[ compstate[list_lines]+BUFFERLINES+1 -gt LINES ]]; then
    compstate[list]='list explanations messages'
    [[ compstate[list_lines]+BUFFERLINES+1 -gt LINES ]] && compstate[list]=''

    toolong='...'
  fi
}

incremental-complete-word "$@"
