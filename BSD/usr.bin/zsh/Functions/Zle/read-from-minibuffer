emulate -L zsh
setopt extendedglob

local opt keys
integer stat

while getopts "k:" opt; do
  case $opt in
    # Read the given number of keys.  This is a bit
    # ropey for more than a single key.
    (k)
    keys=$OPTARG
    ;;

    (*)
    return 1
    ;;
  esac
done
(( OPTIND > 1 )) && shift $(( OPTIND - 1 ))

local readprompt="$1" lbuf_init="$2" rbuf_init="$3"
integer savelim=$UNDO_LIMIT_NO changeno=$UNDO_CHANGE_NO

{
# Use anonymous function to make sure special values get restored,
# even if this function is called as a widget.
# local +h ensures special parameters stay special.
() {
  local pretext="$PREDISPLAY$LBUFFER$RBUFFER$POSTDISPLAY
"
  local +h LBUFFER="$lbuf_init"
  local +h RBUFFER="$rbuf_init"
  local +h PREDISPLAY="$pretext${readprompt:-? }"
  local +h POSTDISPLAY=
  local +h -a region_highlight
  region_highlight=("P${#pretext} ${#PREDISPLAY} bold")

  if [[ -n $keys ]]; then
    zle -R
    read -k $keys
    stat=$?
  else
    local NUMERIC
    unset NUMERIC
    zle split-undo
    UNDO_LIMIT_NO=$UNDO_CHANGE_NO
    zle recursive-edit -K main
    stat=$?
    (( stat )) || REPLY=$BUFFER
  fi
}
} always {
  # This removes the edits relating to the read from the undo history.
  # These aren't useful once we get back to the main editing buffer.
  zle undo $changeno
  UNDO_LIMIT_NO=savelim
}

return $stat
