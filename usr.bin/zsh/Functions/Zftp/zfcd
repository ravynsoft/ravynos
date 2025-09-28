# function zfcd {
# zfcd:  change directory on the remote server.
#
#  Currently has the following features:
# --- an initial string matching $HOME in the directory is turned back into ~
#     to be re-interpreted by the remote server.
# --- zfcd with no arguments changes directory to '~'
# --- `zfcd old new' and `zfcd -' work analagously to cd
# --- if the connection is not currently open, it will try to
#     re-open it with the stored parameters as set by zfopen.
#     If the connection timed out, however, it won't know until
#     too late.  In that case, just try the same zfcd command again
#     (but now `zfcd -' and `zfcd old new' won't work).

# hack: if directory begins with $HOME, turn it back into ~
# there are two reasons for this:
#   first, a ~ on the command line gets expanded even with noglob.
#     (I suppose this is correct, but I wouldn't like to swear to it.)
#   second, we can no do 'zfcd $PWD' and the like, and that will
#     work just as long as the directory structures under the home match.

emulate -L zsh
[[ $curcontext = :zf* ]] || local curcontext=:zfcd

if [[ $1 = /* ]]; then
  zfautocheck -dn || return 1
else
  zfautocheck -d || return 1
fi

if [[ $1 = $HOME || $1 = $HOME/* ]]; then
  1="~${1#$HOME}"
fi

if (( $# == 0 )); then
  # Emulate `cd' behaviour
  set -- '~'
elif [[ $# -eq 1 && $1 = - ]]; then
  # Emulate `cd -' behaviour.
  set -- $zfconfig[lastdir_$ZFTP_SESSION]
elif [[ $# -eq 2 ]]; then
  # Emulate `cd old new' behaviour.
  # We have to find a character not in $1 or $2; ! is a good bet.
  eval set -- "\${ZFTP_PWD:s!$1!$2!}"
fi

# We have to remember the current directory before changing it
# if we want to keep it.
local lastdir=$ZFTP_PWD

if zftp cd "$@" && [[ $lastdir != $ZFTP_PWD ]]; then
  # Invalidate current directory listing.
  rm -f $zfconfig[curdir_$ZFTP_SESSION]
  zfconfig[lastdir_$ZFTP_SESSION]=$lastdir
fi

print $zfconfig[lastloc_$ZFTP_SESSION]
# }
