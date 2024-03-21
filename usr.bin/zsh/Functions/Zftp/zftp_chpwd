# function zftp_chpwd {
# You may want to alter chpwd to call this when $ZFTP_USER is set.

# If the directory really changed...
if [[ $ZFTP_PWD != $zfconfig[lastdir_$ZFTP_SESSION] ]]; then
  # ...and also empty the stored directory listing cache.
  # As this function is called when we close the connection, this
  # is the only place we need to do these two things.
  local curdir=$zfconfig[curdir_$ZFTP_SESSION]
  [[ -n $curdir && -f $curdir ]] && rm -f $curdir
  zfconfig[otherargs_$ZFTP_SESSION]=
  zffcache -d
fi

if [[ -z $ZFTP_USER ]]; then
  # last call, after an FTP logout

  # delete the non-current cached directory
  [[ -n $zfotherdir && -f $zfotherdir ]] && rm -f $zfotherdir

  # don't keep lastdir between opens (do keep lastloc)
  zfconfig[lastdir_$ZFTP_SESSION]=

  # return the display to standard
  zstyle -t ":zftp$curcontext" chpwd && chpwd
else
  [[ -n $ZFTP_PWD ]] && zfconfig[lastdir_$ZFTP_SESSION]=$ZFTP_PWD
  zfconfig[lastloc_$ZFTP_SESSION]="$ZFTP_HOST:$ZFTP_PWD"
  zfconfig[lastuser_$ZFTP_SESSION]="$ZFTP_USER"
  local args
  if [[ -t 1 && -t 2 ]] && zstyle -t ":zftp$curcontext" titlebar; then
    local str=$zfconfig[lastloc_$ZFTP_SESSION]
    [[ ${#str} -lt 70 ]] && str="%m: %~  $str"
    case $TERM in
      sun-cmd) print -Pn "\033]l$str\033\\"
	       ;;
      *xterm*|rxvt|dtterm|Eterm|kterm) print -Pn "\033]2;$str\a"
	     ;;
    esac
  fi
fi
# }
