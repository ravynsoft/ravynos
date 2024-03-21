# function zftransfer {
# Transfer files between two distinct sessions. No remote globbing
# is done, since only single pairs can be transferred.

emulate -L zsh

[[ $curcontext = :zf* ]] || local curcontext=:zftransfer
local sess1 sess2 file1 file2 oldsess=${ZFTP_SESSION}

if [[ $# -ne 2 ]]; then
  print "Usage: zftransfer sess1:file1 sess2:file2" 1>&2
  return 1
fi

if [[ $1 = *:* ]]; then
  sess1=${1%%:*}
  file1=${1#*:}
fi
: ${sess1:=$ZFTP_SESSION}

if [[ $2 = *:* ]]; then
  sess2=${2%%:*}
  file2=${2#*:}
fi
: ${sess2:=$ZFTP_SESSION}
if [[ -z $file2 || $file2 = */ ]]; then
  file2="${file2}${file1:t}"
fi

if [[ $sess1 = $sess2 ]]; then
  print "zftransfer: must use two distinct sessions." 1>&2
  return 1
fi

zftp session $sess1
zfautocheck || return 1

# It's more useful to show the progress for the second part
# of the pipeline, but unfortunately that can't necessarily get
# the size from the pipe --- and if it does, it's probably wrong.
# To avoid that, try to get the size and set it for the progress to
# see.
local style
zstyle -s ':zftp:zftransfer' progress style
if [[ -n $style && $style != none ]]; then
  local ZFTP_TSIZE array
  () {
    zftp remote $file1 >|$1 2>/dev/null
    array=($(<$1))
  } =(<<<'temporary file')
  [[ $#array -eq 2 ]] && ZFTP_TSIZE=$array[1]
fi

# We do the RHS of the pipeline in a subshell, too, so that
# the LHS can get SIGPIPE when it exits.
{ zstyle '*' progress none
  zftp get $file1 } |
( zftp session $sess2
  zfautocheck && zftp put $file2 )

local stat=$?

zftp session $oldsess

return $stat
# }
