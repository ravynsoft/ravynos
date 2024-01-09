# function zfuget {
# Get a list of files from the server with update.
# In other words, only retrieve files which are newer than local
# ones.  This depends on the clocks being adjusted correctly
# (i.e. if one is fifteen minutes out, for the next fifteen minutes
# updates may not be correctly calculated).  However, difficult
# cases --- where the files are the same size, but the remote is newer,
# or have different sizes, but the local is newer -- are prompted for.
#
# Files are globbed on the remote host --- assuming, of course, they
# haven't already been globbed local, so use 'noglob' e.g. as
# `alias zfuget="noglob zfuget"'.
#
# Options:
#  -G    Glob:     turn off globbing
#  -v    verbose:  print more about the files listed.
#  -s    silent:   don't ask, just guess.  The guesses are:
#                - if the files have different sizes but remote is older ) grab
#                - if they have the same size but remote is newer        )
#                  which is safe if the remote files are always the right ones.
#   -t   time:     update the local file times to the same time as the remote.
#                  Currently this only works if you have the `perl' command,
#                  and that perl is version 5 with the standard library.
#                  See the function zfrtime for more gory details.

emulate -L zsh

[[ $curcontext = :zf* ]] || local curcontext=:zfuget
local loc rem locstats remstats doit
local rstat remlist opt opt_v opt_s opt_G opt_t
integer stat do_close

zfuget_print_time() {
  local tim=$1
  print -n "$tim[1,4]/$tim[5,6]/$tim[7,8] $tim[9,10]:$tim[11,12].$tim[13,14]"
  print -n GMT
}

zfuget_print () {
  print -n "\nremote $rem ("
  zfuget_print_time $remstats[2]
  print -n ", $remstats[1] bytes)\nlocal $loc ("
  zfuget_print_time $locstats[2]
  print ", $locstats[1] bytes)"
}

while getopts :vsGt opt; do
  [[ $opt = "?" ]] && print "zfuget: bad option: -$OPTARG" >&2 && return 1
  eval "opt_$opt=1"
done
(( OPTIND > 1 )) && shift $(( OPTIND - 1 ))

zfautocheck || return 1

for remlist in $*; do
  # zfcd directory hack to put the front back to ~
  if [[ $remlist == $HOME || $remlist == $HOME/* ]]; then
    remlist="~${remlist#$HOME}"
  fi
  if [[ $opt_n != 1 ]]; then
    zfrglob remlist
  fi
  if (( $#remlist )); then
    for rem in $remlist; do
      loc=${rem:t}
      doit=y
      remstats=()
      if [[ -f $loc ]]; then
	() {
	  zftp local $loc >|$1
	  locstats=($(<$1))
	  zftp remote $rem >|$1
	  rstat=$?
	  remstats=($(<$1))
	} =(<<<'temporary file')
	if [[ $rstat = 2 ]]; then
	  print "Server does not implement full command set required." 1>&2
	  return 1
	elif [[ $rstat = 1 ]]; then
	  print "File not found on server: $rem" 1>&2
	  stat=1
	  continue
	fi
	[[ $opt_v = 1 ]] && zfuget_print
	if (( $locstats[1] != $remstats[1] )); then
	  # Files have different sizes
	  if [[ $locstats[2] > $remstats[2] && $opt_s != 1 ]]; then
	    [[ $opt_v != 1 ]] && zfuget_print
	    print "Local file $loc more recent than remote," 1>&2
	    print -n "but sizes are different.  Transfer anyway [y/n]? " 1>&2
	    read -q doit
	  fi
	else
	  # Files have same size
	  if [[ $locstats[2] < $remstats[2] ]]; then
	    if [[ $opt_s != 1 ]]; then
	      [[ $opt_v != 1 ]] && zfuget_print
	      print "Local file $loc has same size as remote," 1>&2
	      print -n "but local file is older. Transfer anyway [y/n]? " 1>&2
	      read -q doit
	    fi
	  else
	    # presumably same file, so don't get it.
	    [[ $opt_v = 1 ]] && print Not transferring
	    doit=n
	  fi
	fi
      else
	[[ $opt_v = 1 ]] && print New file $loc
      fi
      if [[ $doit = y ]]; then
	if zftp get $rem >$loc; then
	  if [[ $opt_t = 1 ]]; then
	    # if $remstats is set, it's second element is the remote time
	    zfrtime $loc $rem $remstats[2]
	  fi
	else
	  stat=$?
	fi
	
      fi
    done
  fi
done

(( do_close )) && zfclose

return $stat
# }
