# function zfgcp {
# ZFTP get as copy:  i.e. first arguments are remote, last is local.
# Supposed to work exactly like a normal copy otherwise, i.e.
#  zfgcp rfile lfile
# or
#  zfgcp rfile1 rfile2 rfile3 ... ldir
# Options:
#   -G   don't to remote globbing, else do
#   -t   update the local file times to the same time as the remote.
#        Currently this only works if you have the `perl' command,
#        and that perl is version 5 with the standard library.
#        See the function zfrtime for more gory details.
#
# If there is no current connection, try to use the existing set of open
# parameters to establish one and close it immediately afterwards.

emulate -L zsh

[[ $curcontext = :zf* ]] || local curcontext=:zfgcp
local opt remlist rem loc opt_G opt_t
integer stat do_close

while getopts :Gt opt; do
  [[ $opt = '?' ]] && print "zfgcp: bad option: -$OPTARG" >&2 && return 1
  eval "opt_$opt=1"
done
(( OPTIND > 1 )) && shift $(( OPTIND - 1 ))

zfautocheck || return 1

# hmm, we should really check this after expanding the glob,
# but we shouldn't expand the last argument remotely anyway.
if [[ $# -gt 2 && ! -d $argv[-1] ]]; then
  print "zfgcp:  last argument must be a directory." 2>&1
  return 1
elif [[ $# == 1 ]]; then
  print "zfgcp:  not enough arguments." 2>&1
  return 1
fi

if [[ -d $argv[-1] ]]; then
  local dir=$argv[-1]
  argv[-1]=
  for remlist in $*; do
    # zfcd directory hack to put the front back to ~
    if [[ $remlist = $HOME || $remlist = $HOME/* ]]; then
      remlist="~${remlist#$HOME}"
    fi
    if [[ $opt_G != 1 ]]; then
      zfrglob remlist
    fi
    if (( $#remlist )); then
      for rem in $remlist; do
	loc=$dir/${rem:t}
	if zftp get $rem >$loc; then
	  [[ $opt_t = 1 ]] && zfrtime $rem $loc
	else
	  stat=1
	fi
      done
    fi
  done
else
  if [[ $1 = $HOME || $1 = $HOME/* ]]; then
    1="~${1#$HOME}"
  fi
  zftp get $1 >$2 || stat=$?
fi

(( $do_close )) && zfclose

return $stat
# }
