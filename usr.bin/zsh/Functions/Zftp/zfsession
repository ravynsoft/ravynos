# function zfsession {
# Change or list the sessions for the current zftp connection.

emulate -L zsh

[[ $curcontext = :zf* ]] || local curcontext=:zfsession
local opt opt_l opt_v opt_o opt_d hadopts

while getopts ":lovd" opt; do
  [[ $opt = "?" ]] && print "zfsession: bad option: -$OPTARG" >&2 && return 1
  eval "opt_$opt=1"
  hadopts=1
done
(( OPTIND > 1 )) && shift $(( OPTIND - 1 ))

if [[ $# -gt 1 || (( -n $hadopts && -z $opt_d ) && $# -gt 0 ) ]]
then
  print "Usage: zfsession ( [ -lvod ] | session )" 1>&2
  return 1
fi

if [[ -n $opt_v ]]; then
  local sess
  for sess in $(zftp session); do
    print -n "${(r.15.. ..:.)sess}\t${zfconfig[lastloc_$sess]:-not connected}"
    if [[ $sess = $ZFTP_SESSION ]]; then
      print " *"
    else
      print
    fi
  done
elif [[ -n $opt_l ]]; then
  zftp session
fi

if [[ -n $opt_o ]]; then
  if [[ $zfconfig[lastsession] != $ZFTP_SESSION ]]; then
    local cursession=$ZFTP_SESSION
    zftp session $zfconfig[lastsession]
    zfconfig[lastsession]=$cursession
    print $ZFTP_SESSION
  else
    print "zfsession: no previous session." >&2
    return 1
  fi
fi

if [[ -n $opt_d ]]; then
  local del=${1:-$ZFTP_SESSION} key
  key=${zfconfig[fcache_$del]}
  [[ -n $key ]] && unset $key
  for key in fcache lastloc lastdir curdir otherdir otherargs lastuser; do
    unset "zfconfig[${key}_${del}]"
  done
  zftp rmsession $del
  return
fi

[[ -n $hadopts ]] && return $stat

if [[ $# = 0 ]]; then
  print $ZFTP_SESSION
  return
fi

local oldsession=${ZFTP_SESSION:-default}
zftp session $1
if [[ $ZFTP_SESSION != $oldsession ]]; then
  zfconfig[lastsession]=$oldsession
  zftp_chpwd
fi
# }
