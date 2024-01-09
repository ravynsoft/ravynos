# function zfuput {
# Put a list of files from the server with update.
# See zfuget for details.
#
# Options:
#  -v    verbose:  print more about the files listed.
#  -s    silent:   don't ask, just guess.  The guesses are:
#                - if the files have different sizes but remote is older ) grab
#                - if they have the same size but remote is newer        )
#                  which is safe if the remote files are always the right ones.

emulate -L zsh

[[ $curcontext = :zf* ]] || local curcontext=:zfuput
local loc rem locstats remstats doit
local rstat opt opt_v opt_s
integer stat do_close

zfuput_print_time() {
  local tim=$1
  print -n "$tim[1,4]/$tim[5,6]/$tim[7,8] $tim[9,10]:$tim[11,12].$tim[13,14]"
  print -n GMT
}

zfuput_print () {
  print -n "\nremote $rem ("
  zfuput_print_time $remstats[2]
  print -n ", $remstats[1] bytes)\nlocal $loc ("
  zfuput_print_time $locstats[2]
  print ", $locstats[1] bytes)"
}

while getopts :vs opt; do
  [[ $opt = "?" ]] && print "zfuget: bad option: -$OPTARG" >&2 && return 1
  eval "opt_$opt=1"
done
(( OPTIND > 1 )) && shift $(( OPTIND - 1 ))

zfautocheck || return 1

if [[ $ZFTP_VERBOSE = *5* ]]; then
  # should we turn it off locally?
  print "Messages with code 550 are harmless." >&2
fi

for rem in $*; do
  loc=${rem:t}
  doit=y
  remstats=()
  if [[ ! -f $loc ]]; then
    print "$loc: file not found" >&2
    stat=1
    continue
  fi
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
    [[ $opt_v = 1 ]] && print New file $loc
  else
    [[ $opt_v = 1 ]] && zfuput_print
    if (( $locstats[1] != $remstats[1] )); then
      # Files have different sizes
      if [[ $locstats[2] < $remstats[2] && $opt_s != 1 ]]; then
	[[ $opt_v != 1 ]] && zfuput_print
	print "Remote file $rem more recent than local," 1>&2
	print -n "but sizes are different.  Transfer anyway [y/n]? " 1>&2
	read -q doit
      fi
    else
      # Files have same size
      if [[ $locstats[2] > $remstats[2] ]]; then
	if [[ $opt_s != 1 ]]; then
	  [[ $opt_v != 1 ]] && zfuput_print
	  print "Remote file $rem has same size as local," 1>&2
	  print -n "but remote file is older. Transfer anyway [y/n]? " 1>&2
	  read -q doit
	fi
      else
	# presumably same file, so don't get it.
	[[ $opt_v = 1 ]] && print Not transferring
	doit=n
      fi
    fi
  fi
  if [[ $doit = y ]]; then
    zftp put $rem <$loc || stat=$?
  fi
done

(( do_close )) && zfclose

return $stat
# }
