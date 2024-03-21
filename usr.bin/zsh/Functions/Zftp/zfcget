# function zfcget {
# Continuation get of files from remote server.
# For each file, if it's shorter here, try to get the remainder from
# over there.  This requires the server to support the REST command
# in the way many do but RFC959 doesn't specify.
# Options:
#   -G   don't to remote globbing, else do
#   -t   update the local file times to the same time as the remote.
#        Currently this only works if you have the `perl' command,
#        and that perl is version 5 with the standard library.
#        See the function zfrtime for more gory details.

emulate -L zsh

[[ $curcontext = :zf* ]] || local curcontext=:zfcget
local loc rem stat=0 opt opt_G opt_t remlist locst remst
local rstat tsize

while getopts :Gt opt; do
  [[ $opt = '?' ]] && print "zfcget: bad option: -$OPTARG" && return 1
  eval "opt_$opt=1"
done
(( OPTIND > 1 )) && shift $(( OPTIND - 1 ))

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
      loc=${rem:t}
      if [[ ! -f $loc ]]; then
	# File does not yet exist
	zftp get $rem >$loc || stat=$?
      else
	# Compare the sizes.
	locst=($(zftp local $loc))
	() {
	  zftp remote $rem >|$1
	  rstat=$?
	  remst=($(<$1))
	} =(<<<'temporary file')
	if [[ $rstat = 2 ]]; then
	  print "Server does not support SIZE command.\n" \
	  "Assuming you know what you're doing..." 2>&1
	  zftp getat $rem $locst[1] >>$loc || stat=$?
	  continue
	elif [[ $rstat = 1 ]]; then
	  print "Remote file not found: $rem" 2>&1
	  continue
	fi
	if [[ $locst[1] -gt $remst[1] ]]; then
	  print "Local file is larger!" 2>&1
	  continue;
	elif [[ $locst[1] == $remst[1] ]]; then
	  print "Files are already the same size." 2>&1
	  continue
	else
	  if zftp getat $rem $locst[1] >>$loc; then
	    [[ $opt_t = 1 ]] && zfrtime $loc $rem $remst[2]
	  else
	    stat=1
	  fi
	fi
      fi
    done
  fi
done

return $stat
# }
