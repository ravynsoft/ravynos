# smart-insert-last-word
# Inspired by Christoph Lange <langec@gmx.de> from zsh-users/3265;
# rewritten to correct multiple-call behavior after zsh-users/3270;
# modified to work with copy-earlier-word after zsh-users/5832.
# Edited further per zsh-users/10881 and zsh-users/10884.
#
# This function as a ZLE widget can replace insert-last-word, like so:
#
#   zle -N insert-last-word smart-insert-last-word
#
# With a numeric prefix, behaves like insert-last-word, except that words
# in comments are ignored when interactive_comments is set.
#
# Otherwise, the rightmost "interesting" word from any previous command is
# found and inserted.  The default definition of "interesting" is that the
# word contains at least one alphabetic character, slash, or backslash.
# This definition can be overridden by use of a style like so:
#
#   zstyle :insert-last-word match '*[[:alpha:]/\\]*'
#
# For example, you might want to include words that contain spaces:
#
#   zstyle :insert-last-word match '*[[:alpha:][:space:]/\\]*'
#
# Or include numbers as long as the word is at least two characters long:
#
#   zstyle :insert-last-word match '*([[:digit:]]?|[[:alpha:]/\\])*'
#
# That causes redirections like "2>" to be included.
#
# Note also that the style is looked up based on the widget name, so you
# can bind this function to different widgets to use different patterns:
#
#   zle -N insert-last-assignment smart-insert-last-word
#   zstyle :insert-last-assignment match '[[:alpha:]][][[:alnum:]]#=*'
#   bindkey '\e=' insert-last-assignment
#
# The "auto-previous" style, if set to a true value, causes the search to
# proceed upward through the history until an interesting word is found.
# If auto-previous is unset or false and there is no interesting word, the
# last word is returned.

emulate -L zsh
setopt extendedglob nohistignoredups

# Begin by preserving completion suffix if any
zle auto-suffix-retain

# Not strictly necessary:
# (($+_ilw_hist)) || integer -g _ilw_hist _ilw_count _ilw_cursor _ilw_lcursor _ilw_changeno
# (($+_ilw_result)) || typeset -g _ilw_result

integer cursor=$CURSOR lcursor=$CURSOR
local lastcmd pattern numeric=$NUMERIC

# Save state for repeated calls
if (( HISTNO == _ilw_hist && cursor == _ilw_cursor &&
      UNDO_CHANGE_NO == _ilw_changeno )) && [[ $BUFFER == $_ilw_result ]]
then
    NUMERIC=$[_ilw_count+1]
    lcursor=$_ilw_lcursor
else
    NUMERIC=1
    typeset -g _ilw_lcursor=$lcursor
fi
# Handle the up to three arguments of .insert-last-word
if (( $+1 ))
then
    if (( $+3 )); then
	((NUMERIC = -($1)))
    else
	((NUMERIC = _ilw_count - $1))
    fi
    (( NUMERIC )) || LBUFFER[lcursor+1,cursor+1]=''
    numeric=$((-(${2:--numeric})))
fi
typeset -g _ilw_hist=$HISTNO
typeset -g _ilw_count=$NUMERIC

if [[ -z "$numeric" ]]
then
    zstyle -s :$WIDGET match pattern ||	pattern='*[[:alpha:]/\\]*'
fi

# Note that we must use .up-history for navigation here because of
# possible "holes" in the $history hash (the result of dup expiry).
# We need $history because $BUFFER retains edits in progress as the
# user moves around the history, but we search the unedited lines.

{
  zmodload -i zsh/parameter
  zle .end-of-history              # Start from final command
  zle .up-history || return 1      # Retrieve previous command
  local buffer=$history[$HISTNO]   # Get unedited history line
  lastcmd=( ${${(z)buffer}:#\;} )  # Split into shell words
  if [[ -n "$pattern" ]]
  then
      # This is the "smart" part -- search right-to-left and
      # latest-to-earliest through the history for a word.
      integer n=0 found=$lastcmd[(I)$pattern]
      if zstyle -t :$WIDGET auto-previous
      then
          while (( found == 0 && ++n ))
          do
              zle .up-history || return 1
              buffer=$history[$HISTNO]
              lastcmd=( ${${(z)buffer}:#\;} )
              found=$lastcmd[(I)$pattern]
          done
      fi
      # The following accounts for 1-based index
      (( found-- > 0 && (numeric = $#lastcmd - found) ))
  fi
} always {
  HISTNO=$_ilw_hist                # Return to current command
  CURSOR=$cursor                   # Restore cursor position
  NUMERIC=${numeric:-1}            # In case of fall-through
}

(( NUMERIC > $#lastcmd )) && return 1

LBUFFER[lcursor+1,cursor+1]=$lastcmd[-NUMERIC]
typeset -g _ilw_cursor=$CURSOR _ilw_result=$BUFFER

# This is necessary to update UNDO_CHANGE_NO immediately
zle split-undo && typeset -g _ilw_changeno=$UNDO_CHANGE_NO
