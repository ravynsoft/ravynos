# function zfcd_match {

emulate -L zsh
setopt extendedglob

# see zfcd for details of this hack
if [[ $1 = $HOME || $1 = $HOME/* ]]; then
  1="~${1#$HOME}"
fi

# error messages only
local ZFTP_VERBOSE=45
# should we redirect 2>/dev/null or let the user see it?

local -a match mbegin mend

if [[ $ZFTP_SYSTEM = UNIX* ]]; then
  # hoo, aren't we lucky: this makes things so much easier
  setopt rcexpandparam
  local dir
  if [[ $1 = ?*/* ]]; then
    dir=${1%/*}
  elif [[ $1 = /* ]]; then
    dir=/
  fi
  # If we're using -F, we get away with using a directory
  # to list, but not a glob.  Don't ask me why.
  reply=(${(M)${(f)"$(zftp ls -lF $dir)"}:#d([^[:space:]]##[[:space:]]##)(#c8)?##\/})

  # If ls -lF doesn't work, try dir ...
  if ! (($#reply)); then
    reply=(${(M)${(f)"$(zftp dir $dir)"}:#d([^[:space:]]##[[:space:]]##)(#c8)?##})
  fi
  reply=(${reply/(#b)d([^[:space:]]##[[:space:]]##)(#c8)([^\/]##)\/#/$match[2]})
#  () {
#    zftp ls -LF $dir >|$1
#    reply=($(awk '/\/$/ { print substr($1, 1, length($1)-1) }' $1))
#  } =(<<<'')
  [[ -n $dir && $dir != */ ]] && dir="$dir/"
  if [[ -n $WIDGET ]]; then
    _wanted directories expl 'remote directory' \
        compadd -S/ -q -P "$dir" - $reply
  elif [[ -n $dir ]]; then
    reply=(${dir}$reply)
  fi
else
  # I simply don't know what to do here.
  # Just use the list of files for the current directory.
  zfget_match $*
fi

# }
