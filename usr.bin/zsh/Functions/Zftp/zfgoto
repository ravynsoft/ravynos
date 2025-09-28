# zfgoto bname
# Go to bookmark bname, a location on a remote FTP host.  Unless
# this was the last session or is for anonymous FTP, prompt for
# the user's password.
#
# Maybe this should try and look for an appropriate session to use
# for the transfer.

emulate -L zsh
setopt extendedglob
[[ $curcontext = :zf* ]] || local curcontext=:zfgoto

# Set ZFTP_BMFILE if not already set.  This should agree with
# the corresponding line in zfmark.
: ${ZFTP_BMFILE:=${ZFDOTDIR:-$HOME}/.zfbkmarks}

typeset -A bkmarks
local line opt_n opt

while getopts :n opt; do
  [[ $opt = '?' ]] && print "zfgoto: bad option: -$OPTARG" && return 1
  eval "opt_$opt=1"
done
(( OPTIND > 1 )) && shift $(( OPTIND - 1 ))

if (( $# != 1 )); then
  print "Usage: zfgoto bookmark" >&2
  return 1
fi

if [[ -n $opt_n && -f ~/.ncftp/bookmarks ]]; then
  local oldifs=$IFS
  IFS=,
  while read -rA line; do
    bkmarks[$line[1]]="${line[3]:-anonymous}@${line[2]}:${line[6]}"
  done < ~/.ncftp/bookmarks
  IFS=$oldifs
elif [[ -f $ZFTP_BMFILE ]]; then
  # read in file:  could optimise this by recording last read time
  # comparing with file
  while read -r line; do
    # ignore blank and comment lines
    [[ $line = [[:blank:]]# || $line = [[:blank:]]#'#'* ]] && continue
    bkmarks[${line%% *}]="${line#* }"
  done <$ZFTP_BMFILE
fi

line=${bkmarks[$1]}

if [[ -z $line ]]; then
  print "Bookmark \`$1' not found" >&2
  return 1
fi

local user host dir
user=${line%%@*}
line=${line#*@}
host=${line%%:*}
dir=${line#*:}

if [[ $ZFTP_USER = $user && $ZFTP_HOST = $host ]]; then
  # We're already there, just change directory
  zfcd ${dir:-~}
elif [[ $user = ftp || $user = anonymous ]]; then
  # Anonymous ftp, so we don't need password etc.
  zfanon $host && [[ -n $dir ]] && zfcd $dir
elif [[ $zfconfig[lastloc_$ZFTP_SESSION] = ${host}:* &&
  $user = $zfconfig[lastuser_$ZFTP_SESSION] ]]; then
  # This was the last session, so assume it's still setup in the
  # open parameters
  zfopen && [[ -n $dir ]] && zfcd $dir
else
  # Last try: see if it's in the parameters.
  local params
  params=($(zftp params))
  if [[ $host = $params[1] && $user = $params[2] ]]; then
    zfopen && [[ -n $dir ]] && zfcd $dir
  else
    zfopen $host $user && [[ -n $dir ]] && zfcd $dir
  fi
fi
