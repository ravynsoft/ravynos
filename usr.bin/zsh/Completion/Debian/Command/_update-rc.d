#compdef update-rc.d

local curcontext="$curcontext" state line expl

_arguments -C \
  '-n[show actions without performing them]' \
  '-f[force removal of symlinks]' \
  '1:service:_services' \
  '2:command:(remove defaults start stop)' \
  '*::args:->args' && return

case $words[2] in
  defaults)
    _message -e number 'sequence number'
  ;;
  remove)
    _message 'no more arguments'
  ;;
  st*)
    case ${words[CURRENT-1]} in
      .) _wanted commands expl commands compadd start stop && return ;;
      start|stop) _message -e number 'sequence number' ;;
      *) _message -e runlevels run\ level ;;
    esac
  ;;
esac

return 1
