#autoload

# send-invisible reads a line from the terminal, displaying an
# asterisk for each character typed.  It stores the line in the
# global variable INVISIBLE, and when finished reading, inserts
# the string ${INVISIBLE} into the original command buffer.

# If one argument is given, it is the prompt.  The default is
# "Non-echoed text: "

# If two or three arguments are given, they form the prefix and
# suffix of the inserted INVISIBLE.  Defaults are '${' and '}'
# but these can be replaced, for example with '"${' and '}"' to
# enclose the whole word in double quotes, or '${(z)' and '}' to
# split the value of $INVISIBLE like the shell parser.

# To use:
#  autoload -Uz send-invisible
#  zle -N send-invisible
#  bindkey '^X ' send-invisible
# Or more elaborately:
#  hidden-command() { send-invisible '% ' '${(z)' '}' }
#  zle -N hidden-command
#  bindkey '^X%' hidden-command

# Shamelessly cribbed from read-from-minibuffer.

emulate -L zsh

# Hide the value of INVISIBLE in any output of set and typeset
typeset -g -H INVISIBLE=

local pretext="$PREDISPLAY$LBUFFER$RBUFFER$POSTDISPLAY"$'\n'

# Can't directly make these locals because send-invisible is
# called as a widget, so these special variables are already
# local at the current level and wouldn't get restored
local save_lbuffer=$LBUFFER
local save_rbuffer=$RBUFFER
local save_predisplay=$PREDISPLAY
local save_postdisplay=$POSTDISPLAY
local -a save_region_highlight
save_region_highlight=("${region_highlight[@]}")

{
  local lb rb opn=${2:-'${'} cls=${3:-'}'}
  LBUFFER=
  RBUFFER=
  PREDISPLAY="$pretext${1:-Non-echoed text: }"
  POSTDISPLAY=
  region_highlight=("P${(m)#pretext} ${(m)#PREDISPLAY} bold")

  while zle -R && zle .read-command
  do
    # There are probably more commands that should go into
    # the first clause here to harmlessly beep, because ...
    case $REPLY in
    (send-invisible|run-help|undefined-key|where-is|which-command)
      zle .beep;;
    (push-*|send-break) INVISIBLE=;&
    (accept-*) break;;
    (*)
      LBUFFER=$lb
      RBUFFER=$rb
      zle $REPLY	# ... this could expose something
      lb=$LBUFFER
      rb=$RBUFFER
      INVISIBLE=$BUFFER
	LBUFFER=${(l:$#LBUFFER::*:):-}
	RBUFFER=${(l:$#RBUFFER::*:):-}
      ;;
    esac
  done
} always {
  LBUFFER=$save_lbuffer
  RBUFFER=$save_rbuffer
  PREDISPLAY=$save_predisplay
  POSTDISPLAY=$save_postdisplay
  region_highlight=("${save_region_highlight[@]}")
  zle -R

  # Now that the highlight has been restored with all the old
  # text and cursor positioning, insert the new text.
  LBUFFER+=${INVISIBLE:+${opn}INVISIBLE${cls}}
}
