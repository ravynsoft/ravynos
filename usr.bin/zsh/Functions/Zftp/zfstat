# function zfstat {
# Give a zftp status report using local variables.
# With option -v, connect to the remote host and ask it what it
# thinks the status is.  

setopt localoptions unset
unsetopt ksharrays

[[ $curcontext = :zf* ]] || local curcontext=:zfstat
local i stat=0 opt opt_v

while getopts :v opt; do
  [[ $opt = "?" ]] && print "zfstat: bad option: -$OPTARG" >&2 && return 1
  eval "opt_$opt=1"
done
(( OPTIND > 1 )) && shift $(( OPTIND - 1 ))

[[ -n $ZFTP_SESSION ]] && print "Session:\t$ZFTP_SESSION"

if [[ -n $ZFTP_HOST ]]; then
  print "Host:\t\t$ZFTP_HOST"
  print "Port:\t\t$ZFTP_PORT"
  print "IP:\t\t$ZFTP_IP"
  [[ -n $ZFTP_SYSTEM ]] && print "System type:\t$ZFTP_SYSTEM"
  if [[ -n $ZFTP_USER ]]; then
    print "User:\t\t$ZFTP_USER "
    [[ -n $ZFTP_ACCOUNT ]] && print "Account:\t$AFTP_ACCOUNT"
    print "Directory:\t$ZFTP_PWD"
    print -n "Transfer type:\t"
    if [[ $ZFTP_TYPE = "I" ]]; then
      print Image
    elif [[ $ZFTP_TYPE = "A" ]]; then
      print Ascii
    else
      print Unknown
    fi
    print -n "Transfer mode:\t"
    if [[ $ZFTP_MODE = "S" ]]; then
      print Stream
    elif [[ $ZFTP_MODE = "B" ]]; then
      print Block
    else
      print Unknown
    fi
  else
    print "No user logged in."
  fi
else
  print "Not connected."
  [[ -n $zfconfig[lastloc_$ZFTP_SESSION] ]] &&
  print "Last location:\t$zfconfig[lastloc_$ZFTP_SESSION]"
  stat=1
fi

# things which may be set even if not connected:
[[ -n $ZFTP_REPLY ]] && print "Last reply:\t$ZFTP_REPLY"
print "Verbosity:\t$ZFTP_VERBOSE"
print "Timeout:\t$ZFTP_TMOUT"
print -n "Preferences:\t"
for (( i = 1; i <= ${#ZFTP_PREFS}; i++ )); do
  case $ZFTP_PREFS[$i] in
    [pP]) print -n "Passive "
	  ;;
    [sS]) print -n "Sendport "
	  ;;
    [dD]) print -n "Dumb "
	  ;;
    *) print -n "$ZFTP_PREFS[$i]???"
  esac
done
print

if [[ -n $ZFTP_HOST && $opt_v = 1 ]]; then
  zfautocheck -d
  print "Status of remote server:"
  # make sure we print the reply
  local ZFTP_VERBOSE=045
  zftp quote STAT
fi

return $stat
# }
