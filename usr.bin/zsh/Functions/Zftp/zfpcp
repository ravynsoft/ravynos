# function zfpcp {
# ZFTP put as copy:  i.e. first arguments are remote, last is local.
# Currently only supports
#  zfcp lfile rfile
# if there are two arguments, or the second one is . or .., or ends
# with a slash
# or
#  zfcp lfile1 lfile2 lfile3 ... rdir
# if there are more than two (because otherwise it doesn't
# know if the last argument is a directory on the remote machine).
# However, if the remote machine plays ball by telling us `Is a directory'
# when we try to copy to a directory, zfpcp will then try to do the correct
# thing.

emulate -L zsh

[[ $curcontext = :zf* ]] || local curcontext=:zfpcp
local rem loc
integer stat do_close

zfautocheck || return 1

if [[ $# -gt 2 || $2 = (.|..) || $2 = */ ]]; then
  local dir=$argv[-1]
  argv[-1]=
  # zfcd directory hack to put the front back to ~
  if [[ $dir = $HOME || $dir = $HOME/* ]]; then
    dir="~${dir#$HOME}"
  fi
  [[ -n $dir && $dir != */ ]] || dir="$dir/"
  for loc in $*; do
    rem=$dir${loc:t}
    zftp put $rem <$loc || stat=1
  done
else
  if [[ $2 = $HOME || $2 = $HOME/* ]]; then
    2="~${2#$HOME}"
  fi
  zftp put $2 <$1
  stat=$?
  if [[ stat -ne 0 && $ZFTP_CODE = 553 && $ZFTP_REPLY = *'Is a directory'* ]]
  then
       zftp put $2/$1:t <$1
       stat=$?
  fi
fi

(( $do_close )) && zfclose

return $stat
# }
