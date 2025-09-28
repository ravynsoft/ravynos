# Put standard xterm/dtterm window control codes in shell parameters for
# easy use.  Note that some terminals do not support all combinations.

# autoload -Uz xtermctl ; xtermctl
# xtermctl --explain

# Run once to set up; implements two functions:
#     xterm-tell control [args]
#     xterm-ask control
# See xtermseq below for valid control names.  -ask returns values in
# $reply except for label and title which are returned in $REPLY.

# E.g. one way to maximize the window (see caveat below):
#     xterm-ask screen_chars
#     xterm-tell size_chars $reply

# Might there be terminfo names for these controls?
typeset -Ag xtermseq
xtermseq=(
  1      deiconify             2    iconify
 '3;X;Y' position
 '4;H;W' size_pixels
  5      raise                 6    lower
  7      refresh
 '8;H;W' size_chars
 '9;0'   unmaximize           '9;1' maximize
 11      get_iconify
 13      get_position
 14      get_size_pixels
 18      get_size_chars
 19      get_screen_chars
 20      get_label
 21      get_title
)

local k
for k in ${(k)xtermseq}; do xtermseq[${xtermseq[$k]}]=$k; done

# TODO (maybe): Populate hashes with completed control sequences similar
# to the $fg and $bg mappings created by Functions/Misc/colors

function xterm-tell {
  local seq=${${${xtermseq[$1]:?no such control}/[HX]/$2}/[WY]/$3}
  print -nr -- $'\e['"${seq}"t
}

# The following use of "read -st 2 ..." with the control sequences in
# the prompt string requires zsh 4.3.5-dev-1 or later (zsh-users/12600
# or equivalent patch).

function xterm-ask {
  local esc
  unset REPLY reply
  1=get_${1#get_}
  local seq=${xtermseq[$1]:?no such control}
  case $1 in
  (get_(label|title))
      read -st 2 -rk 3 esc$'?\e['"${seq}"t || return 1
      read -srd $'\e'
      read -srk 1 esc
      ;;
  (get_*)
      read -st 2 -rk 2 esc$'?\e['"${seq}"t || return 1
      IFS=';' read -Arsd t
      (( $#reply > 2 )) && shift reply
      ;;
  esac
  return 0
}

local documentation; read -rd $'\e' documentation <<'EOdoc' <<<$'\e'

CSI = "control sequence introducer": ESC [
OSC = "operating system command": ESC ]
ST = "string terminator": ESC backslash
Ps = "parameter string": (see list below)

All control sequences described here begin with CSI and end with "t".
Note that there are no spaces in control sequences or responses,
except possibly for the text responses for label and title; spaces
shown below are for readability.

Window manipulation (from dtterm, as well as extensions). These
controls may be disabled using the allowWindowOps resource. Valid
values for the first (and any additional parameters) are:

Ps = 1 -> De-iconify window.
Ps = 2 -> Iconify window.
Ps = 3 ; x ; y -> Move window to [x, y].
Ps = 4 ; height ; width -> Resize the xterm window in pixels.
Ps = 5 -> Raise the xterm window to the front of the stacking order.
Ps = 6 -> Lower the xterm window to the bottom of the stacking order.
Ps = 7 -> Refresh the xterm window.
Ps = 8 ; height ; width -> Resize the text area in characters.
Ps = 9 ; 0 -> Restore maximized window.
Ps = 9 ; 1 -> Maximize window (i.e., resize to screen size).
Ps = 1 1 -> Report xterm window state.
             If the xterm window is open (non-iconified), returns CSI 1 t .
             If the xterm window is iconified, returns CSI 2 t .
Ps = 1 3 -> Report xterm window position as CSI 3 ; x; yt
Ps = 1 4 -> Report xterm window in pixels as CSI 4 ; height ; width t
Ps = 1 8 -> Report size of text area as CSI 8 ; height ; width t
Ps = 1 9 -> Report size of screen in characters as CSI 9 ; height ; width t
Ps = 2 0 -> Report xterm window's icon label as OSC L label ST
Ps = 2 1 -> Report xterm window's title as OSC l title ST
Ps >= 2 4 -> Resize to Ps lines (DECSLPP)

The size of the screen in characters is often reported inaccurately.

Gnome-terminal as of v2.16 responds to 13/14/18/19 but fails to insert
the Ps digit 3/4/8/9 between the CSI and the reported dimensions, and
does not appear to respond to any of Ps in 1-9.  Window managers may
also affect behavior; the Gnome desktop allows xterm to resize or
iconify itself but won't reliably let it reposition itself.

EOdoc

[[ -n "${(M)@:#--explain}" ]] && print "$documentation"

return 0
