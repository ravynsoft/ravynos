# function zfput {
# Simple put:  dump every file under the same name, but stripping
# off any directory parts to get the remote filename (i.e. always
# goes into current remote directory).  Use zfpcp to specify new
# file name or new directory at remote end.
#
# -r means put recursively:  any directories encountered will have
#    all their contents to arbitrary depth transferred.  Note that
#    this creates the required directories.  Any files in subdirectories
#    whose names begin with a `.' will also be included.

emulate -L zsh

[[ $curcontext = :zf* ]] || local curcontext=:zfput
local opt opt_r
integer stat do_close abort

while getopts :r opt; do
  [[ $opt = '?' ]] && print "zfget: bad option: -$OPTARG" && return 1
  eval "opt_$opt=1"
done
(( OPTIND > 1 )) && shift $(( OPTIND - 1 ))

zfautocheck

zfput_sub() {
  local subdirs loc rem
  integer stat
  subdirs=()

  for loc in $*; do
    if [[ -n $opt_r ]]; then
      if [[ -d $loc ]]; then
	subdirs=($subdirs $loc)
	continue
      else
	rem=$loc
      fi
    else
      rem=${loc:t}
    fi

    zftp put $rem <$loc
    (( $? )) && stat=$?
    if ! zftp test; then
      abort=1
      (( stat )) || stat=1
      break;
    fi
  done

  while (( $#subdirs  && ! abort )); do
    zftp mkdir ${subdirs[1]}
    zfput_sub ${subdirs[1]}/*(ND)
    (( $? )) && stat=$?
    shift subdirs
  done

  return $stat
}

zfput_sub $*
stat=$?

(( $do_close )) && zfclose

return $stat
# }
