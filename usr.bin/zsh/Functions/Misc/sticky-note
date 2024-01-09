#!/bin/zsh -fi
# A zsh sticky-note ("post-it") application.  Load this file as a function:
#    autoload -Uz sticky-note
#
# It may then be bound as a widget:
#    zle -N sticky-note
# And/or run as a command:
#    sticky-note
#    sticky-note -b
#    sticky-note -l ...
# The -b option is like "zed -b": it installs keymaps/bindings only.
# Use the -l option to list previous sticky notes.  Most options of the
# "fc -l" command are supported, for selecting which notes to display.
# If "sticky-note -l" is run from inside a widget, the cursor is moved
# to the top left of the terminal before display and returned to its
# original position after display.  The -l option is implicitly added
# when sticky-note is called from zle-line-init, to avoid inadvertently
# trapping the user inside the note editor.
#
# Otherwise, invoke the line editor with the previous notes available
# as an editor history.  Two quick taps on the return/enter key finish
# the note, or you can use ^X^W as usual (ZZ in vicmd mode).

# The application is configured by three zstyles, all using the context
# ":sticky-note".  The first two styles are "notefile" and "maxnotes"
# to name the file in which notes are stored and the maximum number of
# notes to retain:
#   zstyle :sticky-note notefile ~/.zsticky
#   zstyle :sticky-note maxnotes 1000

# The "theme" style may be set to control the appearance of the notes.
# The style is an associative array; the current set of values (defaults
# in parens) are:
#   bg    => name or ANSI escape for background color (yellow)
#   fg    => name or ANSI escape for foreground color (black)
#   color => ANSI escape for color scheme ($theme[bg]$theme[fg])
#   reset => ANSI escape to restore "normal" colors
# Values given as names are looked up in the $bg and $fg arrays from the
# "colors" function.  If a "color" field is set, the "bg" and "fg" fields
# are not used.  Example:
#   zstyle :sticky-note theme \
#     bg red \
#     fg $fg_bold[yellow]

# For backwards compatibility with an earlier version, the notefile may
# also be named by the STICKYFILE variable (defaults to $HOME/.zsticky).
# The number of notes stored may be given by STICKYSIZE (1000).

# I encourage all you creative people to contribute enhancements ...

emulate -LR zsh
setopt nobanghist extendedhistory histignoredups

local STICKYFILE=${STICKYFILE:-$HOME/.zsticky}
local STICKYSIZE=${STICKYSIZE:-1000}
local sticky stickyfile stickysize

zstyle -s :sticky-note notefile stickyfile || stickyfile=$STICKYFILE
zstyle -s :sticky-note maxnotes stickysize || stickysize=$STICKYSIZE

# Set up keybindings (adapted from "zed")
if ! bindkey -M sticky >& /dev/null
then
  bindkey -N sticky main
  bindkey -M sticky ^X^W accept-line
  bindkey -M sticky ^M^M accept-line	# Two quick RETs ends note
  bindkey -M sticky ^M self-insert-unmeta
fi
if ! bindkey -M sticky-vicmd >& /dev/null 
then
  bindkey -N sticky-vicmd vicmd
  bindkey -M sticky-vicmd ZZ accept-line
fi

[[ "$1" == -b ]] && return 0

# Look up color theme
local -A theme
(($+bg && $+fg)) || { autoload -Uz colors; colors }
zstyle -m :sticky-note theme '*' || {
    zstyle :sticky-note theme bg yellow fg black
}
zstyle -a :sticky-note theme theme
(( ${+bg[$theme[bg]]} )) && theme[bg]=$bg[$theme[bg]]
(( ${+fg[$theme[fg]]} )) && theme[fg]=$fg[$theme[fg]]
(( ${+theme[color]} )) || theme[color]=$theme[bg]$theme[fg]
(( ${+theme[reset]} )) || theme[reset]=$reset_color

# If invoked as a widget, behave a bit like run-help
if zle
then
  zmodload -i zsh/parameter
  if [[ $* == -*l* || $functrace == *zle-line-init:* ]]
  then
    fc -ap $stickyfile $stickysize $stickysize
    echoti sc
    echoti home
    print -nr "$theme[color]"
    fc -l "${@:--1}" | while read -r sticky; do print -- "$sticky"; done
    print -nr "$theme[reset]"
    echoti rc
  elif [[ $CONTEXT = (cont|select|vared) ]]
  then
    zle -M "No stickies during ${${(z)PREBUFFER}[1]:-$CONTEXT}, sorry"
    zle .beep
    zle -R
  else
    zle .push-line
    BUFFER=sticky-note
    zle .accept-line
  fi
  return 0
fi

# Invoked as a command, behave like zed, but write a history file
fc -ap $stickyfile $stickysize $stickysize

# With a -l option, list the existing sticky notes
if [[ "$*" == -*l* ]]
then
  print -nr "$theme[color]"
  # Use read/print loop to interpolate "\n" in history lines
  fc -f "$@" | while read -r sticky; do print -- "$sticky"; done
  print -nr "$theme[reset]"
  return 0
fi

# Edit a new sticky note and add it to the stickyfile
while vared -h -p "%{$theme[color]%}" -M sticky -m sticky-vicmd sticky
do
  {
    [[ -n "$sticky" ]] && print -s -- "$sticky"
  } always {
    (( TRY_BLOCK_ERROR = 0 ))
  } && break
  echo -n -e '\a'
done
return 0
