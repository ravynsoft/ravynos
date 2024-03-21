# This set of functions implements a sort of magic history searching.
# After predict-on, typing characters causes the editor to look backward
# in the history for the first line beginning with what you have typed so
# far.  After predict-off, editing returns to normal for the line found.
# In fact, you often don't even need to use predict-off, because if the
# line doesn't match something in the history, adding a key performs
# standard completion --- though editing in the middle is liable to delete
# the rest of the line.
#
# With the function based completion system (which is needed for this),
# you should be able to type TAB at almost any point to advance the cursor
# to the next "interesting" character position (usually the end of the
# current word, but sometimes somewhere in the middle of the word).  And
# of course as soon as the entire line is what you want, you can accept
# with RETURN, without needing to move the cursor to the end first.
#
# To use it:
#   autoload -Uz predict-on
#   zle -N predict-on
#   zle -N predict-off
#   bindkey '...' predict-on
#   bindkey '...' predict-off
# Note that all functions are defined when you first type the predict-on
# key, which means typing the predict-off key before that gives a harmless
# error message.

predict-on() {
  zle -N self-insert insert-and-predict
  zle -N magic-space insert-and-predict
  zle -N backward-delete-char delete-backward-and-predict
  zle -N delete-char-or-list delete-no-predict
  zstyle -t :predict verbose && zle -M predict-on
  return 0
}
predict-off() {
  zle -A .self-insert self-insert
  zle -A .magic-space magic-space
  zle -A .backward-delete-char backward-delete-char
  zstyle -t :predict verbose && zle -M predict-off
  return 0
}
insert-and-predict () {
  setopt localoptions noshwordsplit noksharrays

  if [[ $LBUFFER == *$'\012'* ]] || (( PENDING ))
  then
    # Editing a multiline buffer or pasting in a chunk of text;
    # it's unlikely prediction is wanted
    zstyle -t ":predict" toggle && predict-off
    zle .$WIDGET "$@"
    return
  elif [[ ${RBUFFER[1]} == ${KEYS[-1]} ]]
  then
    # Same as what's typed, just move on
    ((++CURSOR))
  else
    LBUFFER="$LBUFFER$KEYS"
    if [[ $LASTWIDGET == (self-insert|magic-space|backward-delete-char) ||
	  $LASTWIDGET == (complete-word|accept-*|predict-*|zle-line-init) ]]
    then
      if ! zle .history-beginning-search-backward
      then
	RBUFFER=""
	if [[ ${KEYS[-1]} != ' ' ]]
	then
	  unsetopt automenu recexact
	  integer curs=$CURSOR pos nchar=${#LBUFFER//[^${KEYS[-1]}]}
	  local -a +h comppostfuncs
	  local crs curcontext="predict:${${curcontext:-:::}#*:}"

	  comppostfuncs=( predict-limit-list )
	  zle complete-word
	  # Decide where to leave the cursor. The dummy loop is used to
	  # get out of that `case'.
	  repeat 1
	  do
	    zstyle -s ":predict" cursor crs
	    case $crs in
	    (complete)
	      # At the place where the completion left it, if it is after
	      # the character typed.
	      [[ ${LBUFFER[-1]} = ${KEYS[-1]} ]] && break
	      ;&
	    (key)
	      # Or maybe at the n'th occurrence of the character typed.
	      pos=${BUFFER[(in:nchar:)${KEYS[-1]}]}
	      if [[ pos -gt curs ]]
	      then
	        CURSOR=$pos
	        break
	      fi
	      ;&
	    (*)
	      # Or else at the previous position.
	      CURSOR=$curs
	    esac
	  done
	fi
      fi
    else
      zstyle -t ":predict" toggle && predict-off
    fi
  fi
  return 0
}
delete-backward-and-predict() {
  if (( $#LBUFFER > 1 ))
  then
    setopt localoptions noshwordsplit noksharrays
    # When editing a multiline buffer, it's unlikely prediction is wanted;
    # or if the last widget was e.g. a motion, then probably the intent is
    # to actually edit the line, not change the search prefix.
    if [[ $LBUFFER = *$'\012'* ||
	  $LASTWIDGET != (self-insert|magic-space|backward-delete-char) ]]
    then
      zstyle -t ":predict" toggle && predict-off
      LBUFFER="$LBUFFER[1,-2]"
    else
      ((--CURSOR))
      zle .history-beginning-search-forward || RBUFFER=""
      return 0
    fi
  else
    zle .kill-whole-line
  fi
}
delete-no-predict() {
  [[ $WIDGET != delete-char-or-list || -n $RBUFFER ]] && predict-off
  zle .$WIDGET "$@"
}

# This is a helper function for autocompletion to prevent long lists
# of matches from forcing a "do you wish to see all ...?" prompt.

predict-limit-list() {
  if (( compstate[list_lines]+BUFFERLINES > LINES ||
	( compstate[list_max] != 0 &&
	    compstate[nmatches] > compstate[list_max] ) ))
  then
    compstate[list]=''
  elif zstyle -t ":predict" list always
  then
    compstate[list]='force list'
  fi
}

# Handle zsh autoloading conventions

[[ -o kshautoload ]] || predict-on "$@"
