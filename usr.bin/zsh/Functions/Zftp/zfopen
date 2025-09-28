# function zfopen {
# Use zftp params to set parameters for open, rather than sending
# them straight to open.  That way they are stored for a future open
# command.
#
# With option -1 (just this 1ce), don't do that.

emulate -L zsh

[[ $curcontext = :zf* ]] || local curcontext=:zfopen
local opt dir opt_1 setparams

while getopts :1 opt; do
  [[ $opt = "?" ]] && print "zfopen: bad option: -$OPTARG" >&2 && return 1
  eval "opt_$opt=1"
done
(( OPTIND > 1 )) && shift $(( OPTIND - 1 ))

# This is where we should try and do same name-lookupage in
# both .netrc and .ncftp/bookmarks .  We could even try saving
# the info in their for new hosts, like ncftp does.

if [[ $1 = */* ]]; then
  1=${1##ftp://}
  dir=${1#*/}
  1=${1%%/*}
fi

if [[ $opt_1 = 1 ]]; then
  zftp open $* || return 1
  if [[ $# = 1 ]]; then
    if ! zftp login; then
      zftp close
      return 1
    fi
  fi
else
  # set parameters, but only if there was at least a host
  (( $# > 0 )) && zfparams $* && setparams=1
  # now call with no parameters
  if ! zftp open; then
    [[ -n $ZFTP_HOST ]] && zftp close
    [[ -n $setparams ]] && zfparams -
    return 1
  fi
fi

if [[ -n $dir ]]; then
  zfcd $dir
fi
# }
