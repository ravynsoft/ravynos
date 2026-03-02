integer start=$1 stop=$2
shift 2

[[ -o zle ]] && zle -I
print -r "$*"

local -a cmd
zmodload -i zsh/parameter || return

# Use xmessage to display the message if the start and stop time
# are the same, indicating we have been scheduled to display it.
# Don't do this if there's already an notification/message for the same user.
# HERE: this should be configurable and we should be able to do
# better if xmessage isn't available, e.g. wish.
if [[ -n $DISPLAY &&  $start -eq $stop ]]; then
  if [[ -n ${commands[kdialog]} && -n $KDE_SESSION_UID ]]
  then
    # We're in a KDE session, most probably.
    # Simple:
    cmd=(kdialog --msgbox)
    # Alternative:
    #  calendar_knotify_show() {
    #    dcop knotify default notify calendar zsh "$*" '' '' 2 0
    #  }
    #  cmd=(calendar_knotify_show)
  elif [[ -n ${commands[xmessage]} ]]; then
    cmd=(xmessage -center)
  fi
  if [[ -n $cmd[1] ]] &&
    ! ps -u$UID | grep $cmd[1] >/dev/null 2>&1; then
    # turn off job control for this
    ($cmd "$*" &)
  fi
fi

return 0
