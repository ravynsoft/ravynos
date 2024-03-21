# function zfget {
# Get files from remote server.  Options:
#   -c   cat: dump files to stdout.
#          alias zfcat="zfget -c"
#          zfpage() { zfget -c "$@" | eval $PAGER }
#        are sensible things to do, but aren't done for you.  Note the
#        second doesn't work on all OS's.
#   -G   don't to remote globbing, else do
#   -t   update the local file times to the same time as the remote.
#        Currently this only works if you have the `perl' command,
#        and that perl is version 5 with the standard library.
#        See the function zfrtime for more gory details.  This has
#        no effect with the -c option.
#
# If the connection is not currently open, try to open it with the current
# parameters (set by a previous zfopen or zfparams), then close it after
# use.  The file is put in the current directory (i.e. using the basename
# of the remote file only); for more control, use zfgcp.

emulate -L zsh

[[ $curcontext = :zf* ]] || local curcontext=:zfget
local loc rem opt remlist opt_G opt_t opt_c
integer stat do_close

while getopts :Gtc opt; do
  [[ $opt = '?' ]] && print "zfget: bad option: -$OPTARG" && return 1
  eval "opt_$opt=1"
done
(( OPTIND > 1 )) && shift $(( OPTIND - 1 ))

zfautocheck || return 1

for remlist in $*; do
  # zfcd directory hack to put the front back to ~
  if [[ $remlist == $HOME || $remlist == $HOME/* ]]; then
    remlist="~${remlist#$HOME}"
  fi
  if [[ $opt_G != 1 ]]; then
    zfrglob remlist
  fi
  if (( $#remlist )); then
    for rem in $remlist; do
      if [[ -n $opt_c ]]; then
	zftp get $rem
	stat=$?
      else
	loc=${rem:t}
	if zftp get $rem >$loc; then
	  [[ $opt_t = 1 ]] && zfrtime $rem $loc
	else
	  stat=1
	fi
      fi
    done
  fi
done

(( $do_close )) && zfclose

return $stat
# }
